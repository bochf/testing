
declare -ri QCRV_PASS=0     # All is good
declare -ri QCRV_SKIP=1     # Test not applicable, considered a PASS
declare -ri QCRV_WARN=2     # Test passed, but should be checked further
declare -ri QCRV_FAIL=3     # Test failed
declare -ri QCRV_BAIL=4     # Test failed so bad that we should stop checks
declare -ri QCRV_IERR=5     # Internal (QC coding) error
declare -ri QCRV_NOPF=6     # Missing policy file
declare -ri QCRV_BADP=7     # Bad policy file - not in expected format

declare -ri QCVL_BSILENT=0  # BL Silent (show results to stdout ONLY)
declare -ri QCVL_REGULAR=1  # Regular verbosity (print all stdout messages)
declare -ri QCVL_SUMMARY=2  # Show "qc_conr" PASS/FAIL only

# Derived / State items
declare -i QCDI_ISBL=-1     # Is run from BL, 0=yes, 1=no, -1=unknown
                            # See IsRunFromBL() below

# -----------------------------------------------------------------------------
# Name:
# Description:
# Paramaters:
# Returns:
# Side effects:
# Notes: If ${QC_WRAPPER_IS_BL} is set, it will then set QCDI_ISBL. This
#        prevents two calls to ps.
function IsRunFromBL
{
    # If we know, then return known value
    if (( QCDI_ISBL != -1 )) ; then return ${QCDI_ISBL} ; fi

    # I have no way of telling at this time. The expectation is that some
    # environmental variable will be set.
    local myparentpid=$(ps -p $$ -o ppid --no-headers)

    if [[ -z ${myparentpid} ]]
    then
        # Uh-oh. Can't determine our parentage!
        return -1
    fi

    local myparent=$(ps -p ${myparentpid} -o comm --no-headers)
    
    if [[ -z ${myparent} ]]
    then
        # Uh-oh. Gotta be some oddness going on here
        return -1
    fi

    # NOTE: I have no clue what the parent BL process would be
    #       Use the following line to find that
    #echo "DEBUG: parent process is ${myparent}" >&2

    # set to bbksh to test BL behaviour locally
    if [[ ${myparent} == *rscd* ]]
    then
        QCDI_ISBL=0
        return 0
    fi

    QCDI_ISBL=1
    return 1
}

# -----------------------------------------------------------------------------
# Name:
# Description:
# Paramaters:
# Returns:
# Side effects:
# Notes:
function VPrintHeader
{
    # There are three options here:
    #                  stdout             logfile
    #   QCVL_BSILENT   Nothing            Header
    #   QCVL_REGULAR   Header             Header
    #   QCVL_SUMMARY   LHS_description    Header


    local header_string=${1}________________________________________________________________________________
    local summary_string=${1}

    # Handle the log file output - unconditionally the same
    echo ${header_string:0:79} >> ${QC_RESULTS_FILE}

    # Nothing to stdout if silent
    if (( ${QC_VERBOSITY} == ${QCVL_BSILENT} ))
    then
        return
    fi

    if (( ${QC_VERBOSITY} == ${QCVL_REGULAR} ))
    then
        echo ${header_string:0:79}
        return
    fi

    if (( ${QC_VERBOSITY} == ${QCVL_SUMMARY} ))
    then
        printf "  %-60s[    ]\b\b\b\b\b" "${summary_string:0:59}"
        return
    fi
}

# -----------------------------------------------------------------------------
# Name:
# Description:
# Paramaters:
# Returns:
# Side effects:
# Notes:
function VPrint
{
    # Always log
    echo $* >> ${QC_RESULTS_FILE}

    if IsRunFromBL
    then
        return 0
    fi

    # Only print when regular verbosity
    if (( ${QC_VERBOSITY} == ${QCVL_REGULAR} ))
    then
        echo $*
    fi

    return 0
}

# -----------------------------------------------------------------------------
# Name:
# Description:
# Paramaters:
# Returns:
# Side effects:
# Notes: MUST use "$@" to preserve positional paramaters!
function VPrintf
{
    # Always log
    printf "$@" >> ${QC_RESULTS_FILE}

    if IsRunFromBL
    then
        return 0
    fi

    # Only print when regular verbosity
    if (( ${QC_VERBOSITY} == ${QCVL_REGULAR} ))
    then
        printf "$@"
    fi

    return 0
}

# -----------------------------------------------------------------------------
# Name:
# Description:
# Paramaters:
# Returns:
# Side effects:
# Notes: The original design had this function returning success/failure.
#        Now it exits with explicit codes. The issues at hand are:
#         - Function (return) works good for independent use
#         - Exiting works good for ease of test development
#        Because all tests should be wrapped *somehow*, the thinking was that
#        the exit() approach (with no error message) was best.
function SourceGlobalConfig
{
    # NOTE: Not (really) defined at this time
    local global_config_file=${1:-runcfg/QCGlobal.conf}

    if [[ ! -e ${global_config_file} ]]
    then
        # Don't print, let the exception roll up.
        # This is really about calling the test independently as the wrapper
        # is really responsible for this. Checks will alwasys be wrapped
        QCExit ${QCRV_NOGC}
    fi

    # Source our configuration
    . ${global_config_file}

    # Check for all the requied/expected values
    # See notes on error reporting for file -e test above
    if [[ -z ${QC_GROUP} ]]        ; then QCExit ${QCRV_BMGC} ; fi
    if [[ -z ${QC_RESULTS_FILE} ]] ; then QCExit ${QCRV_BMGC} ; fi
    if [[ -z ${QC_VERBOSITY} ]]    ; then QCExit ${QCRV_BMGC} ; fi

    # This is optional - And it is unnessary to do the -z test if we use
    #   the double bracket syntax
    if [[ ${QC_WRAPPER_IS_BL} == true ]]
    then
        # Set this based upon global config "hint". This saves two "ps"
        # calls per check.
        QCDI_ISBL=0
    fi

    if [[ ${QC_WRAPPER_IS_BL} == false ]]
    then
        # See the above note
        QCDI_ISBL=1
    fi


    return 0
}

# -----------------------------------------------------------------------------
# Name:
# Description:
# Paramaters:
# Returns:
# Side effects:
# Notes: A single group specification of "always" insures that this test will
#        always run (in all groups). This is intended for a rare set of
#        reserved tests - use with caution in general tests.
function GroupCheck
{
    # The QC_GROUP setting must be set
    if [[ -z ${QC_GROUP} ]]
    then
        QCExit ${QCRV_IERR}
    fi

    # Force string compare only because we want to do both at once
    if [[ $# == "1" &&  $1 == "always" ]]
    then
        VPrint "This test runs in all groups"
        return 0
    fi

    local valid_group=

    # The paramaters are the names of the groups that this test will run in
    for valid_group in $*
    do
        if [[ ${QC_GROUP} == ${valid_group} ]]
        then
            VPrint "Test running in a supported group."
            return 0
        fi
    done

    # If we are here, then we are running in an unsupported group.
    VPrint "Test skipped based on called group considerations."
    QCExit ${QCRV_SKIP}
}

# -----------------------------------------------------------------------------
# Name:
# Description:
# Paramaters:
# Returns:
# Side effects:
# Notes: * The *validity* of the override is NOT checked - intentionally
#        * The date and other override data *should* be pulled from the 
#          override file, that has not been defined or supported... yet.
function OverrideCheck
{
    local override_number=$0

    override_number=${override_number##*QC}
    override_number=${override_number%_*}

    if [[ -e overrides/QC${override_number} ]]
    then
        VPrint "This test has a temporary override in place."
        QCExit ${QCRV_SKIP}
    fi

    # No override found.
    return 0
}

# -----------------------------------------------------------------------------
# Name:
# Description:
# Paramaters:
# Returns:
# Side effects:
# Notes:
function QCExit
{
    # Use param, or consider missing exit code an error
    local -i evalue=${1:-$QCRV_IERR}

    if IsRunFromBL
    then
        case ${evalue}
        in
        ${QCRV_PASS})
           echo "Value1=PASS" ;;
        ${QCRV_SKIP})
           echo "Value1=SKIP" ;;
        ${QCRV_WARN})
           echo "Value1=WARN" ;;
        ${QCRV_FAIL})
           echo "Value1=FAIL" ;;
        ${QCRV_BAIL})
           echo "Value1=BAIL" ;;
        ${QCRV_IERR})
           echo "Value1=IERR"
           exit ${QCRV_IERR} ;;
        ${QCRV_NOPF})
           echo "Value1=NOPF"
           exit ${QCRV_NOPF} ;;
        ${QCRV_BADP})
           echo "Value1=BADP"
           exit ${QCRV_BADP} ;;
        ${QCRV_OVRR})
           echo "Value1=OVRR" ;;
        ${QCRV_NOGC})
           echo "Value1=NOGC"
           exit ${QCRV_NOGC} ;;
        ${QCRV_BMGC})
           echo "Value1=BMGC"
           exit ${QCRV_BMGC} ;;
        *)
           echo "Value1=IERR" 
           exit ${QCRV_IERR} ;;
        esac

        # Blade logic expects 0, regardless of 
        exit 0
    fi

    if (( ${QC_VERBOSITY} == ${QCVL_SUMMARY} ))
    then
        # A second case statement is used because:
        # - The output is slightly different
        # - The desire is to leave the ANSI option open
        case ${evalue}
        in
        ${QCRV_PASS})
           echo "PASS" ;;
        ${QCRV_SKIP})
           echo "SKIP" ;;
        ${QCRV_WARN})
           echo "WARN" ;;
        ${QCRV_FAIL})
           echo "FAIL" ;;
        ${QCRV_BAIL})
           echo "BAIL" ;;
        ${QCRV_IERR})
           echo "IERR" ;;
        ${QCRV_NOPF})
           echo "NOPF" ;;
        ${QCRV_BADP})
           echo "BADP" ;;
         ${QCRV_OVRR})
           echo "OVRR" ;;
         ${QCRV_NOGC})
           echo "NOGC" ;;
         ${QCRV_BMGC})
           echo "BMGC" ;;
       *)
           echo "IERR" ;;
        esac


    fi



    exit ${evalue}
}




declare was_successfully_sourced=true
