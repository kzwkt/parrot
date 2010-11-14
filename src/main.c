/*
Copyright (C) 2007-2010, Parrot Foundation.

=head1 NAME

src/main.c - The PIR/PASM compiler frontend to libparrot

=head1 DESCRIPTION

Start Parrot

=head2 Functions

=over 4

=cut

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parrot/api.h"


/* HEADERIZER HFILE: none */

/* HEADERIZER BEGIN: static */
/* Don't modify between HEADERIZER BEGIN / HEADERIZER END.  Your changes will be lost. */

static void help(void);
static void help_debug(void);
PARROT_WARN_UNUSED_RESULT
PARROT_PURE_FUNCTION
static int is_all_digits(ARGIN(const char *s))
        __attribute__nonnull__(1);

PARROT_WARN_UNUSED_RESULT
PARROT_PURE_FUNCTION
static int is_all_hex_digits(ARGIN(const char *s))
        __attribute__nonnull__(1);

static void Parrot_version(void);
PARROT_CAN_RETURN_NULL
static const char * parseflags(
    PMC *interp,
    int argc,
    ARGIN(const char *argv[]),
    ARGOUT(int *pgm_argc),
    ARGOUT(const char ***pgm_argv),
    ARGMOD(Parrot_Run_core_t *core),
    ARGMOD(Parrot_trace_flags *trace))
        __attribute__nonnull__(3)
        __attribute__nonnull__(4)
        __attribute__nonnull__(5)
        __attribute__nonnull__(6)
        __attribute__nonnull__(7)
        FUNC_MODIFIES(*pgm_argc)
        FUNC_MODIFIES(*pgm_argv)
        FUNC_MODIFIES(*core)
        FUNC_MODIFIES(*trace);

static void parseflags_minimal(
    Parrot_Init_Args *initargs,
    int argc,
    ARGIN(const char *argv[]))
        __attribute__nonnull__(3);

static void usage(ARGMOD(FILE *fp))
        __attribute__nonnull__(1)
        FUNC_MODIFIES(*fp);

#define ASSERT_ARGS_help __attribute__unused__ int _ASSERT_ARGS_CHECK = (0)
#define ASSERT_ARGS_help_debug __attribute__unused__ int _ASSERT_ARGS_CHECK = (0)
#define ASSERT_ARGS_is_all_digits __attribute__unused__ int _ASSERT_ARGS_CHECK = (\
       PARROT_ASSERT_ARG(s))
#define ASSERT_ARGS_is_all_hex_digits __attribute__unused__ int _ASSERT_ARGS_CHECK = (\
       PARROT_ASSERT_ARG(s))
#define ASSERT_ARGS_Parrot_version __attribute__unused__ int _ASSERT_ARGS_CHECK = (0)
#define ASSERT_ARGS_parseflags __attribute__unused__ int _ASSERT_ARGS_CHECK = (\
       PARROT_ASSERT_ARG(argv) \
    , PARROT_ASSERT_ARG(pgm_argc) \
    , PARROT_ASSERT_ARG(pgm_argv) \
    , PARROT_ASSERT_ARG(core) \
    , PARROT_ASSERT_ARG(trace))
#define ASSERT_ARGS_parseflags_minimal __attribute__unused__ int _ASSERT_ARGS_CHECK = (\
       PARROT_ASSERT_ARG(argv))
#define ASSERT_ARGS_usage __attribute__unused__ int _ASSERT_ARGS_CHECK = (\
       PARROT_ASSERT_ARG(fp))
/* Don't modify between HEADERIZER BEGIN / HEADERIZER END.  Your changes will be lost. */
/* HEADERIZER END: static */

/*

=item C<int main(int argc, const char *argv[])>

The entry point from the command line into Parrot.

=cut

*/

int
main(int argc, const char *argv[])
{
    int          stacktop;
    const char  *sourcefile;
    PMC         *interp;
    PMC         *bytecodepmc;
    PMC         *argsarray;
    int          status;
    int          pir_argc;
    const char **pir_argv;
    Parrot_Init_Args *initargs = (Parrot_Init_Args*)calloc(sizeof(Parrot_Init_Args));

    Parrot_Run_core_t  core  = PARROT_SLOW_CORE;
    Parrot_trace_flags trace = PARROT_NO_TRACE;

    /* internationalization setup */
    /* setlocale(LC_ALL, ""); */
    PARROT_BINDTEXTDOMAIN(PACKAGE, LOCALEDIR);
    PARROT_TEXTDOMAIN(PACKAGE);

    /* Parse minimal subset of flags */
    parseflags_minimal(initargs, argc, argv);

    if (!(Parrot_api_make_interpreter(NULL, PARROT_NO_FLAGS, initargs, &interp) &&
          Parrot_api_set_executable_name(interp, argv[0])) {
        fprintf(stderr, "PARROT VM: Could not initialize new interpreter");
        exit(EXIT_FAILURE);
    }

    /* Parse flags */
    sourcefile = parseflags(interp, argc, argv, &pir_argc, &pir_argv, &core, &trace);

    Parrot_api_set_runcore(interp, (Parrot_Run_core_t) core, trace);

    if (imcc_run(raw, interp, sourcefile, argc, argv, &bytecodepmc)) {
        if (!(Parrot_api_build_argv_array(interp, pir_argc, pir_argv, &argsarray) &&
              Parrot_api_run_bytecode(interp, bytecodepmc, argsarray))) {
            fprintf(stderr, "PARROT VM: Could not load or run bytecode)
            exit(EXIT_FAILURE);
        }
    }

    /* Clean-up after ourselves */
    Parrot_api_destroy_interpreter(interp);
}

/*

=item C<static int is_all_digits(const char *s)>

Tests all characters in a string are decimal digits.
Returns 1 if true, 0 as soon as a non-decimal found

=cut

*/

PARROT_WARN_UNUSED_RESULT
PARROT_PURE_FUNCTION
static int
is_all_digits(ARGIN(const char *s))
{
    ASSERT_ARGS(is_all_digits);
    for (; *s; ++s)
        if (!isdigit((unsigned char)*s))
            return 0;
    return 1;
}

/*

=item C<static int is_all_hex_digits(const char *s)>

Tests all characters in a string are hexadecimal digits.
Returns 1 if true, 0 as soon as a non-hex found

=cut

*/

PARROT_WARN_UNUSED_RESULT
PARROT_PURE_FUNCTION
static int
is_all_hex_digits(ARGIN(const char *s))
{
    ASSERT_ARGS(is_all_hex_digits);
    for (; *s; ++s)
        if (!isxdigit(*s))
            return 0;
    return 1;
}

/*

=item C<static void usage(FILE *fp)>

Outputs usage error message.

=cut

*/

static void
usage(ARGMOD(FILE *fp))
{
    ASSERT_ARGS(usage)
    fprintf(fp,
            "parrot -[acEGhprtvVwy.] [-d [FLAGS]] [-D [FLAGS]]"
            "[-O [level]] [-R runcore] [-o FILE] <file>\n");
}

/*

=item C<static void help_debug(void)>

Print out list of debugging flag values.

=cut

*/

static void
help_debug(void)
{
    ASSERT_ARGS(help_debug)
    /* split printf for C89 compliance on string length */
    printf(
    "--imcc-debug -d [Flags] ...\n"
    "    0002    lexer\n"
    "    0004    parser\n"
    "    0008    imc\n"
    "    0010    CFG\n"
    "    0020    optimization 1\n"
    "    0040    optimization 2\n"
    "    0100    AST\n"
    "    1000    PBC\n"
    "    2000    PBC constants\n"
    "    4000    PBC fixups\n"
    "\n");
    printf(
    "--parrot-debug -D [Flags] ...\n"
    "    0001    memory statistics\n"
    "    0002    print backtrace on exception\n"
    "    0004    JIT debugging\n"
    "    0008    interpreter startup\n"
    "    0010    thread debugging\n"
    "    0020    eval/compile\n"
    "    0040    fill I, N registers with garbage\n"
    "    0080    show when a context is destroyed\n"
    "\n"
    "--trace -t [Flags] ...\n"
    "    0001    opcodes\n"
    "    0002    find_method\n"
    "    0004    function calls\n");
}

/*

=item C<static void help(void)>

Print out "help" list of options.

=cut

*/

static void
help(void)
{
    ASSERT_ARGS(help)
    /* split printf for C89 compliance on string length */
    printf(
    "parrot [Options] <file>\n"
    "  Options:\n"
    "    -h --help\n"
    "    -V --version\n"
    "    -I --include add path to include search\n"
    "    -L --library add path to library search\n"
    "       --hash-seed F00F  specify hex value to use as hash seed\n"
    "    -X --dynext add path to dynamic extension search\n"
    "   <Run core options>\n"
    "    -R --runcore slow|bounds|fast\n"
    "    -R --runcore trace|profiling|gcdebug\n"
    "    -t --trace [flags]\n"
    "   <VM options>\n"
    "    -D --parrot-debug[=HEXFLAGS]\n"
    "       --help-debug\n");
    printf(
    "    -w --warnings\n"
    "    -G --no-gc\n"
    "       --gc-threshold=percentage    maximum memory wasted by GC\n"
    "       --gc-debug\n"
    "       --leak-test|--destroy-at-end\n"
    "    -g --gc ms|inf set GC type\n"
    "    -. --wait    Read a keystroke before starting\n"
    "       --runtime-prefix\n"
    "   <Compiler options>\n"
    "    -d --imcc-debug[=HEXFLAGS]\n"
    "    -v --verbose\n"
    "    -E --pre-process-only\n"
    "    -o --output=FILE\n"
    "       --output-pbc\n"
    "    -O --optimize[=LEVEL]\n"
    "    -a --pasm\n"
    "    -c --pbc\n"
    "    -r --run-pbc\n"
    "    -y --yydebug\n"
    "   <Language options>\n"
    "see docs/running.pod for more\n");
}


/*

=item C<static void Parrot_version(void)>

Print out parrot version number.

=cut

*/

static void
Parrot_version(void)
{
    ASSERT_ARGS(Parrot_version)
    printf("This is Parrot version " PARROT_VERSION);
    printf(" built for " PARROT_ARCHNAME ".\n");
    printf("Copyright (C) 2001-2010, Parrot Foundation.\n\
\n\
This code is distributed under the terms of the Artistic License 2.0.\
\n\
For more details, see the full text of the license in the LICENSE file\
\n\
included in the Parrot source tree.\n\n");

    exit(EXIT_SUCCESS);
}

/*

=item C<static void parseflags_minimal(Parrot_Init_Args *initargs, int argc,
const char *argv[])>

Parse minimal subset of args required for initializing interpreter.

=cut

*/
static void
parseflags_minimal(Parrot_Init_Args *initargs, int argc, ARGIN(const char *argv[]))
{
    ASSERT_ARGS(parseflags_minimal)

    int pos = 0;

    while (pos < argc) {
        const char *arg = argv[pos];

        if (STREQ(arg, "--gc")) {
            ++pos;
            if (pos == argc) {
                fprintf(stderr,
                        "main: No GC specified."
                        "\n\nhelp: parrot -h\n");
                exit(EXIT_FAILURE);
            }
            arg = argv[pos];
            initargs->gc_system = arg;
            break;
        }

        /* arg should start with --gc-threshold *and* contain more chars */
        else if (strncmp(arg, "--gc-threshold", 14) == 0) {

            /* the next character could be '=' */
            if (arg[14] == '=') {
                arg++;
            }

            /* or the end of the string... */
            else if (arg[14] == '\0'

            /* and there's another argument */
                 && pos < argc - 1) {
                 arg = argv[++pos];
            }

            /* ANYTHING ELSE IS WRONG */
            else {
                fprintf(stderr, "--gc-threshold needs an argument");
                exit(EXIT_FAILURE);
            }

            if (is_all_digits(arg)) {
                initargs->gc_threshold = strtoul(arg, NULL, 10);

                if (initargs->gc_threshold > 1000) {
                    fprintf(stderr, "error: maximum GC threshold is 1000\n");
                    exit(EXIT_FAILURE);
                }
            }
            else {
                fprintf(stderr, "error: invalid GC threshold specified:"
                        "'%s'\n", arg);
                exit(EXIT_FAILURE);
            }
            ++pos;
            arg = argv[pos];
        }
        else if (!strncmp(arg, "--hash-seed", 11)) {

            if ((arg = strrchr(arg, '=')))
                ++arg;
            else
                arg = argv[++pos];

            if (is_all_hex_digits(arg)) {
                initargs->hash_seed = strtoul(arg, NULL, 16);
            }
            else {
                fprintf(stderr, "error: invalid hash seed specified:"
                        "'%s'\n", arg);
                exit(EXIT_FAILURE);
            }
            ++pos;
            arg = argv[pos];
        }
        ++pos;
    }
}

/*

=item C<static const char * parseflags(PMC *interp, int argc, const char
*argv[], int *pgm_argc, const char ***pgm_argv, Parrot_Int *core, Parrot_Int
*trace)>

Parse Parrot's command line for options and set appropriate flags.

=cut

*/

PARROT_CAN_RETURN_NULL
static const char *
parseflags(PMC *interp,
        int argc, ARGIN(const char *argv[]),
        ARGOUT(int *pgm_argc), ARGOUT(const char ***pgm_argv),
        ARGMOD(Parrot_Int *core), ARGMOD(Parrot_Int *trace))
{
    ASSERT_ARGS(parseflags)
    struct longopt_opt_info opt = LONGOPT_OPT_INFO_INIT;
    int status;
    int result = 1;

    if (argc == 1) {
        usage(stderr);
        exit(EXIT_SUCCESS);
    }

    while ((status = longopt_get(interp, argc, argv, Parrot_cmd_options(), &opt)) > 0) {
        switch (opt.opt_id) {
          case 'R':
            if (STREQ(opt.opt_arg, "slow") || STREQ(opt.opt_arg, "bounds"))
                *core = PARROT_SLOW_CORE;
            else if (STREQ(opt.opt_arg, "fast") || STREQ(opt.opt_arg, "function"))
                *core = PARROT_FAST_CORE;
            else if (STREQ(opt.opt_arg, "jit"))
                *core = PARROT_FAST_CORE;
            else if (STREQ(opt.opt_arg, "exec"))
                *core = PARROT_EXEC_CORE;
            else if (STREQ(opt.opt_arg, "trace"))
                *core = PARROT_SLOW_CORE;
            else if (STREQ(opt.opt_arg, "profiling"))
                *core = PARROT_PROFILING_CORE;
            else if (STREQ(opt.opt_arg, "gcdebug"))
                *core = PARROT_GC_DEBUG_CORE;
            else {
                fprintf(stderr,
                        "main: Unrecognized runcore '%s' specified."
                        "\n\nhelp: parrot -h\n", opt.opt_arg);
                exit(EXIT_FAILURE);
            }
            break;
          case 'g':
            /* Handled in parseflags_minimal */
            break;
          case OPT_GC_THRESHOLD:
            /* handled in parseflags_minimal */
            break;
          case 't':
            if (opt.opt_arg && is_all_hex_digits(opt.opt_arg)) {
                const unsigned long _temp = strtoul(opt.opt_arg, NULL, 16);
                const Parrot_trace_flags _temp_flag = (Parrot_trace_flags)_temp;
                *trace = _temp_flag;
            }
            else
                *trace = PARROT_TRACE_OPS_FLAG;
            break;
          case 'D':
            if (opt.opt_arg && is_all_hex_digits(opt.opt_arg))
                Parrot_api_debug_flag(interp, strtoul(opt.opt_arg, NULL, 16), 1);
            else
                Parrot_api_debug_flag(interp, PARROT_MEM_STAT_DEBUG_FLAG, 1);
            break;

          case '.':  /* Give Windows Parrot hackers an opportunity to
                      * attach a debuggger. */
            fgetc(stdin);
            break;
          case 'h':
            help();
            exit(EXIT_FAILURE);
            break;
          case OPT_HASH_SEED:
            /* handled in parseflags_minimal */
            break;
          case OPT_HELP_DEBUG:
            help_debug();
            exit(EXIT_FAILURE);
            break;
          case OPT_RUNTIME_PREFIX:
            Parrot_io_printf(interp, "%Ss\n",
                    Parrot_get_runtime_path(interp));
            exit(EXIT_SUCCESS);
          case 'V':
            Parrot_version();
            break;

          case OPT_GC_DEBUG:
#if DISABLE_GC_DEBUG
            Parrot_warn(interp, PARROT_WARNINGS_ALL_FLAG,
                "PARROT_GC_DEBUG is set but the binary was compiled "
                "with DISABLE_GC_DEBUG.");
#endif
            Parrot_api_flag(interp, PARROT_GC_DEBUG_FLAG, 1);
            break;
          case OPT_DESTROY_FLAG:
            Parrot_api_flag(interp PARROT_DESTROY_FLAG, 1);
            break;
          case 'I':
            result = Parrot_api_add_include_search_path(interp, opt.opt_arg);
            break;
          case 'L':
            result = Parrot_api_add_library_search_path(interp, opt.opt_arg);
            break;
          case 'X':
            result = Parrot_api_add_dynext_search_path(interp, opt.opt_arg);
            break;
          case 'w':
            result = Parrot_api_set_warnings(interp, PARROT_WARNINGS_ALL_FLAG);
            break;
          case 'o':
            result = Parrot_api_set_output_file(interp, opt.opt_arg);
            break;
          case OPT_PBC_OUTPUT:
            result = Parrot_api_set_output_file(interp, NULL);
            break;
          default:
            /* languages handle their arguments later (after being initialized) */
            break;
        }
    }

    if (!result) {
        fprintf(stderr, "Parrot VM: Error parsing option %s\n", argv[opt.opt_index]);
        usage(stderr);
        exit(EXIT_FAILURE);
    }
    if (status == -1) {
        fprintf(stderr, "%s\n", opt.opt_error);
        usage(stderr);
        exit(EXIT_FAILURE);
    }

    /* reached the end of the option list and consumed all of argv */
    if (argc == opt.opt_index) {
        /* We are not looking at an option, so it must be a program name */
        fprintf(stderr, "Missing program name\n");
        usage(stderr);
        exit(EXIT_FAILURE);
    }

    *pgm_argc = argc - opt.opt_index;
    *pgm_argv = argv + opt.opt_index;

    return (*pgm_argv)[0];
}
/*

=back

=head1 SEE ALSO

F<compilers/imcc/main.c>, unfortunately.

=cut

*/

/*
 * Local variables:
 *   c-file-style: "parrot"
 * End:
 * vim: expandtab shiftwidth=4:
 */
