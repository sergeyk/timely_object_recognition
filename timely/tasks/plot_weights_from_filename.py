def plot_weights_from_filename(filename):
    from synthetic.belief_state import BeliefState
    import matplotlib.pyplot as plt
    num_actions = 20 #HACK
    num_feats = BeliefState.num_features
    weights = np.loadtxt(filename)
    W = weights.reshape((num_actions,num_feats))
    plt.clf()
    p = plt.pcolor(W)
    p.axes.invert_yaxis()
    plt.colorbar()
    plt.savefig(filename+'.png')