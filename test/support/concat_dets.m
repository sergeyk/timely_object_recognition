% quick script to test my python nms code
function dets = concat_dets_mc()

addpath('/Users/sergeyk/research/object_detection/voc-release4/sergeyk')

% load a couple of images to construct one matrix
load('/Users/sergeyk/research/timely/data/test_support/dets/000002_dets_all_may25_DP.mat');
dets = concat_dets_mc_helper(dets_mc);
dets = clipboxes_dims(335,500,dets);
dets1 = [dets 0*ones(rows(dets),1)];

load('/Users/sergeyk/research/timely/data/test_support/dets/000003_dets_all_may25_DP.mat');
dets = concat_dets_mc_helper(dets_mc);
dets = clipboxes_dims(500,375,dets);
dets2 = [dets 1*ones(rows(dets),1)];

dets = [dets1; dets2];
save('/Users/sergeyk/research/timely/data/test_support/val_dets.mat','dets');

function dets = concat_dets_mc_helper(dets_mc)

dets = [];
classes = [];
for c=1:20
  pick = nms(dets_mc{c},0.5);
  dets = [dets; dets_mc{c}(pick,:)];
  num_dets = rows(dets_mc{c}(pick,:));
  classes = [classes; repmat(c-1, [num_dets 1])];
end

dets = [dets, classes];
