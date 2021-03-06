

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <math.h>


#include "const.h"
#include "error.h"
#include "cfmask.h"

#include "misc.h"


/*****************************************************************************
MODULE:  prctile

PURPOSE: Calculate Percentile of an integer array

RETURN: SUCCESS
        FAILURE
*****************************************************************************/
int prctile
(
    int16 *array, /* I: input data pointer */
    int nums,     /* I: number of input data array */
    int16 min,    /* I: minimum value in the input data array */
    int16 max,    /* I: maximum value in the input data array  */
    float prct,   /* I: percentage threshold */
    float *result /* O: percentile calculated */
)
{
    int *interval = NULL; /* array to store data in an interval */
    int i, j;           /* loop variables */
    int loops;          /* data range for input data */
    float inv_nums_100; /* inverse of the nums value * 100 */
    int sum;

    /* Just return 0 if no input value */
    if (nums == 0)
    {
        *result = 0.0;
        return SUCCESS;
    }
    else
    {
        *result = max;
    }

    loops = max - min + 1;

    interval = calloc(loops, sizeof(int));
    if (interval == NULL)
    {
        RETURN_ERROR("Invalid memory allocation", "prctile", FAILURE);
    }

    for (i = 0; i < nums; i++)
    {
        interval[array[i] - min]++;
    }

    inv_nums_100 = (1.0 / nums) * 100.0;
    sum = 0;
    for (j = 0; j < loops; j++)
    {
        sum += interval[j];
        if ((sum * inv_nums_100) >= prct)
        {
            *result = min + j;
            break;
        }
        else
        {
            continue;
        }
    }
    free(interval);

    return SUCCESS;
}


/*****************************************************************************
MODULE:  prctile2

PURPOSE:  Calculate Percentile of a floating point array

RETURN: SUCCESS
        FAILURE
*****************************************************************************/
int prctile2
(
    float *array, /* I: input data pointer */
    int nums,     /* I: number of input data array */
    float min,    /* I: minimum value in the input data array */
    float max,    /* I: maximum value in the input data array  */
    float prct,   /* I: percentage threshold */
    float *result /* O: percentile calculated */
)
{
    int *interval;      /* array to store data in an interval */
    int i, j;           /* loop variables */
    int start, end;     /* start/end variables */
    int loops;          /* data range of input data */
    float inv_nums_100; /* inverse of the nums value * 100 */
    int sum;

    /* Just return 0 if no input value */
    if (nums == 0)
    {
        *result = 0.0;
        return SUCCESS;
    }
    else
    {
        *result = max;
    }

    start = (int)rint(min);
    end = (int)rint(max);

    loops = end - start + 2;

    interval = calloc(loops, sizeof(int));
    if (interval == NULL)
    {
        RETURN_ERROR("Invalid memory allocation", "prctile2", FAILURE);
    }

    for (i = 0; i < nums; i++)
    {
        interval[(int)rint(array[i]) - start]++;
    }

    inv_nums_100 = (1.0 / nums) * 100.0;
    sum = 0;
    for (j = 0; j < loops; j++)
    {
        sum += interval[j];
        if ((sum * inv_nums_100) >= prct)
        {
            *result = start + j;
            break;
        }
        else
        {
            continue;
        }
    }
    free(interval);

    return SUCCESS;
}


/*****************************************************************************
MODULE:  get_args

PURPOSE:  Gets the command-line arguments and validates that the required
arguments were specified.

RETURN VALUE:
Type = int
Value           Description
-----           -----------
FAILURE         Error getting the command-line arguments or a command-line
                argument and associated value were not specified
SUCCESS         No errors encountered

NOTES:
  1. Memory is allocated for the input and output files.  All of these should
     be character pointers set to NULL on input.  The caller is responsible
     for freeing the allocated memory upon successful return.
*****************************************************************************/
int get_args
(
    int argc,          /* I: number of cmd-line args */
    char *argv[],      /* I: string of cmd-line args */
    char **xml_infile, /* O: address of input XML filename */
    float *cloud_prob, /* O: cloud_probability input */
    int *cldpix,       /* O: cloud_pixel buffer used for image dilate */
    int *sdpix,        /* O: shadow_pixel buffer used for image dilate */
    bool *use_cirrus,  /* O: use Cirrus data */
    bool *use_thermal, /* O: use Thermal data */
    bool *verbose      /* O: verbose */
)
{
    char FUNC_NAME[] = "get_args"; /* function name */
    int c;                         /* current argument index */
    int option_index;              /* index for the command-line option */
    static int verbose_flag = 0;   /* verbose flag */
    static int cldpix_default = 3; /* Default buffer for cloud pixel dilate */
    static int sdpix_default = 3;  /* Default buffer for shadow pixel dilate */
    static float cloud_prob_default = 22.5; /* Default cloud probability */
    static int use_cirrus_flag = 0;  /* Default to not using Cirrus band data */
    static int use_thermal_flag = 1; /* Default to using Thermal band data */
    char errmsg[MAX_STR_LEN];               /* error message */
    static struct option long_options[] = {
        {"xml", required_argument, 0, 'i'},
        {"without-thermal", no_argument, &use_thermal_flag, 0},
        {"with-cirrus", no_argument, &use_cirrus_flag, 1},
        {"prob", required_argument, 0, 'p'},
        {"cldpix", required_argument, 0, 'c'},
        {"sdpix", required_argument, 0, 's'},
        {"verbose", no_argument, &verbose_flag, 1},
        {"version", no_argument, 0, 'v'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };

    /* Assign the default values */
    *cloud_prob = cloud_prob_default;
    *cldpix = cldpix_default;
    *sdpix = sdpix_default;

    /* Loop through all the cmd-line options */
    opterr = 0; /* turn off getopt_long error msgs as we'll print our own */
    while (1)
    {
        /* optstring in call to getopt_long is empty since we will only
           support the long options */
        c = getopt_long(argc, argv, "", long_options, &option_index);
        if (c == -1)
        {
            /* Out of cmd-line options */
            break;
        }

        switch (c)
        {
        case 0:
            /* If this option set a flag, do nothing else now. */
            if (long_options[option_index].flag != 0)
                break;

        case 'h':          /* help */
            usage();
            exit(SUCCESS);
            break;

        case 'v':          /* version */
            version();
            exit(SUCCESS);
            break;

        case 'i':          /* xml infile */
            *xml_infile = strdup(optarg);
            break;

        case 'p':          /* cloud probability value */
            *cloud_prob = atof(optarg);
            break;

        case 'c':          /* cloud pixel value for image dilation */
            *cldpix = atoi(optarg);
            break;

        case 's':          /* snow pixel value for image dilation */
            *sdpix = atoi(optarg);
            break;

        case '?':
        default:
            sprintf(errmsg, "Unknown option %s", argv[optind - 1]);
            usage();
            RETURN_ERROR(errmsg, FUNC_NAME, FAILURE);
            break;
        }
    }

    /* Make sure the infile was specified */
    if (*xml_infile == NULL)
    {
        sprintf(errmsg, "XML input file is a required argument");
        usage();
        RETURN_ERROR(errmsg, FUNC_NAME, FAILURE);
    }

    /* Check the use cirrus band flag */
    if (use_cirrus_flag)
        *use_cirrus = true;
    else
        *use_cirrus = false;

    /* Check the use thermal band flag */
    if (use_thermal_flag)
        *use_thermal = true;
    else
        *use_thermal = false;

    /* Check the verbose flag */
    if (verbose_flag)
        *verbose = true;
    else
        *verbose = false;

    if (*verbose)
    {
        printf("XML_input_file = %s\n", *xml_infile);
        printf("cloud_probability = %f\n", *cloud_prob);
        printf("cloud_pixel_buffer = %d\n", *cldpix);
        printf("shadow_pixel_buffer = %d\n", *sdpix);
        if (*use_cirrus)
            printf("use_cirrus = true\n");
        else
            printf("use_cirrus = false\n");
        if (*use_thermal)
            printf("use_thermal = true\n");
        else
            printf("use_thermal = false\n");
    }

    return SUCCESS;
}


bool is_leap_year
(
    int year /*I: Year to test */
)
{
    if (((year % 4) != 0) || (((year % 100) == 0) && ((year % 400) != 0)))
        return false;
    else
        return true;
}


/* Calculate day of year given year, month, and day of month */
bool convert_year_month_day_to_doy
(
    int year,  /* I: Year */
    int month, /* I: Month */
    int day,   /* I: Day of month */
    int *doy   /* O: Day of year */
)
{
    /* Days in month for non-leap years */
    static const int noleap[12] =
        {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    /* Days in month for leap years */
    static const int leap[12] =
        {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int i;
    int doy_sum;

    /* Check to make sure month entered is OK */
    if ((month < 1) || (month > 12))
    {
        return false;
    }

    /* Calculate day of year */
    doy_sum = 0;
    if (is_leap_year(year))
    {
        for (i = 0; i < month - 1; i++)
            doy_sum += leap[i];
    }
    else
    {
        for (i = 0; i < month - 1; i++)
            doy_sum += noleap[i];
    }
    doy_sum += day;

    *doy = doy_sum;

    return true;
}
