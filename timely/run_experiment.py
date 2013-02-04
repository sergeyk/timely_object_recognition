#!/usr/bin/env python
"""
Script for running experiments and outputting the results.
"""

import os
import simplejson as json
import glob
import argparse
from skvisutils import Dataset
from skpyutils import mpi, skutil

from timely import DatasetPolicy, Evaluation, TimelyConfig

# TODO: load config properly -- Perhaps take res dir and stuff on the command line?
config = TimelyConfig()


def load_configs(name):
    """
    If name is a file, calls load_config_file(name).
    If it's a directory, calls load_config_file() on every file in it.
    """
    def load_config_file(filename):
        """
        Load the config in json format and return as list of experiments.
        Look for config_dir/#{name}.json
        """
        print("Loading %s" % filename)
        assert(os.path.exists(filename))
        with open(filename) as f:
            cf = json.load(f)

        # Gather multiple values of settings, if given
        num_conditions = 1
        bounds_list = []
        if 'bounds' in cf:
            bounds_list = cf['bounds'] \
                if isinstance(cf['bounds'][0], list) else [cf['bounds']]
            num_conditions *= len(bounds_list)

        if 'policy_mode' in cf:
            cp_modes_list = []
            cp_modes_list = cf['policy_mode'] \
                if isinstance(cf['policy_mode'], list) else [cf['policy_mode']]
            num_conditions *= len(cp_modes_list)

        if 'weights_mode' in cf:
            w_modes_list = cf['weights_mode'] \
                if isinstance(cf['weights_mode'], list) else [cf['weights_mode']]
            num_conditions *= len(w_modes_list)

        if 'rewards_mode' in cf:
            r_modes_list = cf['rewards_mode'] \
                if isinstance(cf['rewards_mode'], list) else [cf['rewards_mode']]
            num_conditions *= len(r_modes_list)

        configs = []
        for i in range(0, num_conditions):
            configs.append(dict(cf))
            if 'bounds' in cf:
                configs[i]['bounds'] = bounds_list[i % len(bounds_list)]
            if 'policy_mode' in cf:
                configs[i]['policy_mode'] = cp_modes_list[
                    i % len(cp_modes_list)]
            if 'weights_mode' in cf:
                configs[i]['weights_mode'] = w_modes_list[
                    i % len(w_modes_list)]
            if 'rewards_mode' in cf:
                configs[i]['rewards_mode'] = r_modes_list[
                    i % len(r_modes_list)]
        return configs

    dirname = os.path.join(config.config_dir, name)
    if name[-4:] == 'json':
        filename = os.path.join(config.config_dir, name)
    else:
        filename = os.path.join(config.config_dir, name + '.json')
    collectname = os.path.join(config.config_dir, name + '.txt')

    if os.path.isdir(dirname):
        filenames = glob.glob(dirname + '/*.json')
        configs = []
        for filename in filenames:
            configs += load_config_file(filename)
    elif os.path.isfile(filename):
        configs = load_config_file(filename)
    elif os.path.isfile(collectname):
        filenames = open(collectname, 'r').readlines()
        configs = []
        for filename in filenames:
            configs += load_config_file(os.path.join(
                config.config_dir, os.path.dirname(name), filename[:-1] + '.json'))
    else:
        raise RuntimeError('NO WEIGHTS FOUND!')
    return configs


def main():
    parser = argparse.ArgumentParser(
        description="Run experiments with the timely detection system.")

    parser.add_argument('--test_dataset',
                        choices=['val', 'test', 'trainval'],
                        default='val',
                        help="""Dataset to use for testing. Run on val until final runs.
        The training dataset is inferred (val->train; test->trainval; trainval->trainval).""")

    parser.add_argument('--first_n', type=int,
                        help='only take the first N images in the test dataset')

    parser.add_argument('--first_n_train', type=int,
                        help='only take the first N images in the train dataset')

    parser.add_argument('--config',
                        help="""Config file name that specifies the experiments to run.
        Give name such that the file is configs/#{name}.json or configs/#{name}/
        In the latter case, all files within the directory will be loaded.""")

    parser.add_argument('--suffix',
                        help="Overwrites the suffix in the config(s).")

    parser.add_argument('--bounds10', action='store_true',
                        default=False, help='set bounds to [0,10]')

    parser.add_argument('--bounds515', action='store_true',
                        default=False, help='set bounds to [5,15]')

    parser.add_argument('--force', action='store_true',
                        default=False, help='force overwrite')

    parser.add_argument('--wholeset_prs', action='store_true',
                        default=False, help='evaluate in the final p-r regime')

    parser.add_argument('--no_apvst', action='store_true',
                        default=False, help='do NOT evaluate in the ap vs. time regime')

    parser.add_argument('--det_configs', action='store_true',
                        default=False, help='output detector statistics to det_configs')

    parser.add_argument('--inverse_prior', action='store_true',
                        default=False, help='use inverse prior class values')

    args = parser.parse_args()
    print(args)

    # If config file is not given, just run one experiment using default config
    if not args.config:
        configs = [DatasetPolicy.default_config]
    else:
        configs = load_configs(args.config)

    # Load the dataset
    dataset = Dataset('full_pascal_' + args.test_dataset)
    if args.first_n:
        dataset.images = dataset.images[:args.first_n]

    # Infer train_dataset
    if args.test_dataset == 'test':
        train_dataset = Dataset('full_pascal_trainval')
    elif args.test_dataset == 'val':
        train_dataset = Dataset('full_pascal_train')
    elif args.test_dataset == 'trainval':
        train_dataset = Dataset('full_pascal_trainval')
    else:
        None  # impossible by argparse settings

    # Only need to set training dataset values; evaluation gets it from there
    if args.inverse_prior:
        train_dataset.set_values('inverse_prior')

    # TODO: hack
    if args.first_n_train:
        train_dataset.images = train_dataset.images[:args.first_n_train]

    # In both the above cases, we use the val dataset for weights
    weights_dataset_name = 'full_pascal_val'

    dets_tables = []
    dets_tables_whole = []
    clses_tables_whole = []
    all_bounds = []

    plot_infos = []
    for config_f in configs:
        if args.suffix:
            config_f['suffix'] = args.suffix
        if args.bounds10:
            config_f['bounds'] = [0, 10]
        if args.bounds515:
            config_f['bounds'] = [5, 15]
        assert(not (args.bounds10 and args.bounds515))
        if args.inverse_prior:
            config_f['suffix'] += '_inverse_prior'
            config_f['values'] = 'inverse_prior'

        dp = DatasetPolicy(
            dataset, train_dataset, weights_dataset_name, **config_f)
        ev = Evaluation(config, dp)
        all_bounds.append(dp.bounds)
        plot_infos.append(dict((k, config_f[k]) for k in (
            'label', 'line', 'color') if k in config_f))
        # output the det configs first
        if args.det_configs:
            dp.output_det_statistics()

        # evaluate in the AP vs. Time regime, unless told not to
        if not args.no_apvst:
            dets_table = ev.evaluate_vs_t(None, None, force=args.force)
            # dets_table_whole,clses_table_whole =
            # ev.evaluate_vs_t_whole(None,None,force=args.force)
            if mpi.mpi.comm_rank == 0:
                dets_tables.append(dets_table)
                # dets_tables_whole.append(dets_table_whole)
                # clses_tables_whole.append(clses_table_whole)

        # optionally, evaluate in the standard PR regime
        if args.wholeset_prs:
            ev.evaluate_detections_whole(None, force=args.force)

    # and plot the comparison if multiple config files were given
    if not args.no_apvst and len(configs) > 1 and mpi.mpi.comm_rank == 0:
        # filename of the final plot is the config file name
        dirname = config.get_evals_dir(dataset.get_name())
        filename = args.config
        if args.inverse_prior:
            filename += '_inverse_prior'

        # det avg
        ff = os.path.join(dirname, '%s_det_avg.png' % filename)
        ff_nl = os.path.join(dirname, '%s_det_avg_nl.png' % filename)

        # make sure directory exists
        skutil.makedirs(os.path.dirname(ff))

        Evaluation.plot_ap_vs_t(dets_tables, ff, all_bounds, with_legend=True,
                                force=True, plot_infos=plot_infos)
        Evaluation.plot_ap_vs_t(dets_tables, ff_nl, all_bounds,
                                with_legend=False, force=True, plot_infos=plot_infos)

        if False:
            # det whole
            ff = os.path.join(dirname, '%s_det_whole.png' % filename)
            ff_nl = os.path.join(dirname, '%s_det_whole_nl.png' % filename)
            Evaluation.plot_ap_vs_t(dets_tables_whole, ff, all_bounds,
                                    with_legend=True, force=True, plot_infos=plot_infos)
            Evaluation.plot_ap_vs_t(dets_tables_whole, ff_nl, all_bounds,
                                    with_legend=False, force=True, plot_infos=plot_infos)

            # cls whole
            ff = os.path.join(dirname, '%s_cls_whole.png' % filename)
            ff_nl = os.path.join(dirname, '%s_cls_whole_nl.png' % filename)
            Evaluation.plot_ap_vs_t(clses_tables_whole, ff, all_bounds,
                                    with_legend=True, force=True, plot_infos=plot_infos)
            Evaluation.plot_ap_vs_t(clses_tables_whole, ff_nl, all_bounds,
                                    with_legend=False, force=True, plot_infos=plot_infos)

if __name__ == '__main__':
    main()
