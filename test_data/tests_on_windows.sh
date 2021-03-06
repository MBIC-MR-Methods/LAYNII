# For internal testing. Just to check whether programs execute.

..\LN2_LAYER_SMOOTH -input sc_VASO_act.nii -layer_file sc_layers.nii -FWHM 1
..\LN_LAYER_SMOOTH -input sc_VASO_act.nii -layer_file sc_layers.nii -FWHM 0.3 -NoKissing

..\LN_BOCO -Nulled lo_Nulled_intemp.nii -BOLD lo_BOLD_intemp.nii -trialBOCO 40 -shift
..\LN_MP2RAGE_DNOISE -INV1 sc_INV1.nii -INV2 sc_INV2.nii -UNI sc_UNI.nii

..\LN2_LAYERS -rim sc_rim.nii -nr_layers 10 -equivol

..\LN_3DCOLUMNS -layers sc_layers_3dcolumns.nii -landmarks sc_landmarks_3dcolumns.nii
..\LN_CORREL2FILES -file1 lo_Nulled_intemp.nii -file2 lo_BOLD_intemp.nii
..\LN_COLUMNAR_DIST -layers sc_layers_3dcolumns.nii -landmarks sc_landmarks.nii
..\LN_DIRECT_SMOOTH -input sc_UNI.nii -FWHM 2 -direction 3
..\LN_EXTREMETR -input lo_BOLD_intemp.nii
..\LN_FLOAT_ME -input lo_BOLD_intemp.nii
..\LN_SHORT_ME -input lo_VASO_act.nii -output short.nii
..\LN_INT_ME -input LN_INT_ME -input lo_BOLD_act.nii
..\LN_GFACTOR -input sc_INV2.nii  -variance 1 -direction 1 -grappa 2 -cutoff 200
..\LN_GRADSMOOTH -gradfile lo_gradT1.nii -input lo_VASO_act.nii -FWHM 1 -within -selectivity 0.1
..\LN_GRADSMOOTH_ITER -gradfile lo_gradT1.nii -input lo_VASO_act.nii -FWHM 1 -within -selectivity 0.1
..\LN_GROW_LAYERS -rim sc_rim.nii
..\LN_INTPRO -image sc_UNI.nii -min -direction 2 -range 3
..\LN_IMAGIRO -layers sc_layers_3dcolumns.nii -columns sc_columns_3dcolumns.nii -data sc_BOLD_act.nii 
..\LN_LEAKY_LAYERS -rim lo_rim_LL.nii
..\LN_NOISEME -input lo_VASO_act.nii -std 1
..\LN_RAGRUG -input sc_rim.nii
..\LN_SKEW -input lo_BOLD_intemp.nii
..\LN_TEMPSMOOTH -input lo_BOLD_intemp.nii -box 1
..\LN_TEMPSMOOTH -input lo_BOLD_intemp.nii -gaus 1
..\LN_TRIAL -input lo_BOLD_intemp.nii -trialdur 20
..\LN_ZOOM -mask sc_layers_3dcolumns.nii -input sc_UNI.nii
..\LN_LOITUMA -equidist sc_distlay_1000.nii -leaky sc_leakylay_1000.nii -FWHM 1 -nr_layers 10
..\LN_NOISE_KERNEL -input lo_Nulled_intemp.nii -kernel_size 7
..\LN2_DEVEIN -layer_file lo_layers.nii -column_file lo_columns.nii -input lo_BOLD_act.nii -ALF lo_ALF.nii
..\LN2_RIMIFY -input sc_rim.nii -innergm 2 -outergm 1 -gm 3 -output rimified_tim.nii
..\LN_INFO -input lo_T1EPI.nii
..\LN_CONLAY -layers lo_sc_layers.nii -ref lo_T1EPI.nii -subsample -output lo_layers_out.nii
..\LN2_COLUMNS -rim sc_rim.nii -midgm sc_midGM.nii -nr_columns 300
