
#include "../dep/laynii_lib.h"

int show_help(void) {
    printf(
    "LN_MP2RAGE_DNOISE : Denoising MP2RAGE data.\n"
    "\n"
    "    This program removes some of the background noise in MP2RAGE,\n"
    "    UNI images to make themn look like MPRAGE images. This is done\n"
    "    without the phase information. See O’Brien K.R. et al. (2014)\n"
    "    Robust T1-Weighted Structural Brain Imaging and Morphometry\n"
    "    at 7T Using MP2RAGE. PLoS ONE 9(6): e99676.\n"
    "    <DOI:10.1371/journal.pone.0099676>\n"
    "\n"
    "Usage:\n"
    "    LN_MP2RAGE_DNOISE -INV1 INV1.nii -INV2 INV2.nii -UNI UNI.nii -beta 0.2\n"
    "    ../LN_MP2RAGE_DNOISE -INV1 sc_INV1.nii -INV2 sc_INV2.nii -UNI sc_UNI.nii \n" 
    "\n"
    "Options\n"
    "    -help   : Show this help.\n"
    "    -INV1   : Nifti (.nii) file of the first inversion time.\n"
    "    -INV2   : Nifti (.nii) file of the second inversion time.\n"
    "    -UNI    : Nifti (.nii) of MP2RAGE UNI. Expecting SIEMENS \n"
    "              unsigned integer 12 values between 0-4095. \n"
    "    -beta   : Regularization term. Default is '0.2'.\n"
    "    -output       : (Optional) Output filename, including .nii or\n"
    "                    .nii.gz, and path if needed. Overwrites existing files.\n"
    "\n"
    "Notes:\n"
    "    An application of this program is mentioned in this blog post:\n"
    "    <https://layerfmri.com/mp2rage>\n"
    );
    return 0;
}

int main(int argc, char* argv[]) {
    bool use_outpath = false;
    char *fout = NULL, *fin1 = NULL, *fin2 = NULL, *fin3 = NULL;
    int ac;
    float SIEMENS_f = 4095.0;  // uint12 range 0-4095
    float beta = 0.2;
    if (argc < 3) return show_help();

    // Process user options
    for (ac = 1; ac < argc; ac++) {
        if (!strncmp(argv[ac], "-h", 2)) {
            return show_help();
        } else if (!strcmp(argv[ac], "-beta")) {
            if (++ac >= argc) {
                fprintf(stderr, "** missing argument for -beta");
                return 1;
            }
            beta = atof(argv[ac]);
        } else if (!strcmp(argv[ac], "-INV1")) {
            if (++ac >= argc) {
                fprintf(stderr, "** missing argument for -INV1\n");
                return 1;
            }
            fin1 = argv[ac];
        } else if (!strcmp(argv[ac], "-INV2")) {
            if (++ac >= argc) {
                fprintf(stderr, "** missing argument for -INV2\n");
                return 1;
            }
            fin2 = argv[ac];
        } else if (!strcmp(argv[ac], "-UNI")) {
            if (++ac >= argc) {
                fprintf(stderr, "** missing argument for -UNI\n");
                return 1;
            }
            fin3 = argv[ac];
            fout = fin3;
        } else if (!strcmp(argv[ac], "-output")) {
            if (++ac >= argc) {
                fprintf(stderr, "** missing argument for -output\n");
                return 1;
            }
            use_outpath = true;
            fout = argv[ac];
        } else {
            fprintf(stderr, "** invalid option, '%s'\n", argv[ac]);
            return 1;
        }
    }

    if (!fin1) {
        fprintf(stderr, "** missing option '-INV1'\n");
        return 1;
    }
    if (!fin2) {
        fprintf(stderr, "** missing option '-INV2'\n");
        return 1;
    }
    if (!fin3) {
        fprintf(stderr, "** missing option '-UNI '\n");
        return 1;
    }

    // Read input dataset
    nifti_image* nii1 = nifti_image_read(fin1, 1);
    if (!nii1) {
        fprintf(stderr, "** failed to read NIfTI from '%s'\n", fin1);
        return 2;
    }
    nifti_image* nii2 = nifti_image_read(fin2, 1);
    if (!nii2) {
        fprintf(stderr, "** failed to read NIfTI from '%s'\n", fin2);
        return 2;
    }
    nifti_image* nii3 = nifti_image_read(fin3, 1);
    if (!nii3) {
        fprintf(stderr, "** failed to read NIfTI from '%s'\n", fin3);
        return 2;
    }

    log_welcome("LN_MP2RAGE_DNOISE");
    log_nifti_descriptives(nii1);
    log_nifti_descriptives(nii2);
    log_nifti_descriptives(nii3);

    // Get dimensions of input
    const int size_x = nii1->nx;
    const int size_y = nii1->ny;
    const int size_z = nii1->nz;
    const int nr_voxels = size_z * size_y * size_x;

    // ========================================================================
    // Fix datatype issues
    nifti_image* nii_inv1 = copy_nifti_as_float32(nii1);
    float* nii_inv1_data = static_cast<float*>(nii_inv1->data);
    nifti_image* nii_inv2 = copy_nifti_as_float32(nii2);
    float* nii_inv2_data = static_cast<float*>(nii_inv2->data);
    nifti_image* nii_uni = copy_nifti_as_float32(nii3);
    float* nii_uni_data = static_cast<float*>(nii_uni->data);

    // Allocate output nifti files
    nifti_image* nii_denoised = copy_nifti_as_float32(nii1);
    float* nii_denoised_data = static_cast<float*>(nii_denoised->data);
    nifti_image* nii_phaseerr = copy_nifti_as_float32(nii1);
    float* nii_phaseerr_data = static_cast<float*>(nii_phaseerr->data);

    // fixing slopes

    // ========================================================================
    // Handle scaling factor effects
    float scl_slope1=nii_inv1->scl_slope;
    float scl_slope2=nii_inv2->scl_slope;
    float scl_slope3=nii_uni->scl_slope;
    if (scl_slope1 != 0 || scl_slope2 != 0 || scl_slope3 != 0) {
        for (int i = 0; i != nr_voxels; ++i) {
            *(nii_inv1_data + i) = *(nii_inv1_data + i) * scl_slope1;
            *(nii_inv2_data + i) = *(nii_inv2_data + i) * scl_slope2;
            *(nii_uni_data  + i) = *(nii_uni_data + i) * scl_slope3;
        }
    } else {
        cout << "    !!!Warning!!! Input nifti header contains scl_scale=0.\n"
             << "    Make sure to check the resulting output image.\n"<< endl;
    }
    // We can set scaling factor to 1 because we have accounted for them above
    nii_denoised->scl_slope = 1.0 ;
    nii_phaseerr->scl_slope = 1.0 ;

    // ========================================================================
    // Big calculation across all voxels
    beta = beta * SIEMENS_f;
    for (int i = 0; i != nr_voxels; ++i) {
        float val_uni = *(nii_uni_data + i);
        float val_inv1 = *(nii_inv1_data + i);
        float val_inv2 = *(nii_inv2_data + i);
        float new_uni1, new_uni2, val_uni_wrong;

        // Skip nan or zero voxels
        if ( val_uni != val_uni || val_uni == 0 || val_uni == 0.0) {
            *(nii_phaseerr_data + i) = 0;
            *(nii_denoised_data + i) = 0;
        } else {
            // Scale UNI to range of -0.5 to 0.5 (as in O’Brien et al. [2014])
            val_uni = val_uni / SIEMENS_f - 0.5;

            if (val_uni < 0) {
                new_uni1 = val_inv2 * (1. / (2. * val_uni)
                                       + sqrt(1. / pow(2 * val_uni, 2) - 1.));
            } else {
                new_uni1 = val_inv2 * (1. / (2. * val_uni)
                                       - sqrt(1. / pow(2 * val_uni, 2) - 1.));
            }

            // Eq. 2 in O’Brien et al. [2014].
            new_uni2 = (new_uni1 * val_inv2 - beta)
                       / ((pow(new_uni1, 2) + pow(val_inv2, 2) + 2. * beta));

            // Scale back the value range
            *(nii_denoised_data + i) =  (new_uni2 + 0.5) * SIEMENS_f;

            // ----------------------------------------------------------------
            // Border enhance
            val_uni_wrong = val_inv1 * val_inv2
                            / (pow(val_inv1, 2) + pow(val_inv2, 2));

            *(nii_phaseerr_data + i) = val_uni_wrong;
            // ----------------------------------------------------------------
        }
    }

    save_output_nifti(fout, "denoised", nii_denoised, true, use_outpath);
    save_output_nifti(fout, "border_enhance", nii_phaseerr, true);

    cout << "  Finished." << endl;
    return 0;
}
