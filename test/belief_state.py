from context import *
from skvisutils import Dataset
from timely.dataset_policy import DatasetPolicy
from timely.belief_state import BeliefState


class TestBeliefState(unittest.TestCase):
    def setUp(self):
        d = Dataset(test_config, 'test_pascal_trainval').load_from_pascal('trainval', force=True)
        d2 = Dataset(test_config, 'test_pascal_test').load_from_pascal('test', force=True)
        config = {'detectors': ['csc_default']}
        self.dp = DatasetPolicy(test_config, d, d2, **config)
        self.bs = BeliefState(d, self.dp.actions)

    def test_featurization(self):
        ff = self.bs.compute_full_feature()
        np.set_printoptions(precision=2)
        print self.bs.block_out_action(ff, -1)
        print self.bs.block_out_action(ff, 0)
        print self.bs.block_out_action(ff, 3)
        # TODO: make asserts here

if __name__ == '__main__':
    unittest.main()
