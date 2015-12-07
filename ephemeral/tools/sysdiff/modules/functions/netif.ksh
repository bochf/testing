
# ----------------------------------------------------------------------------
# up_if_list
# Produces a list of plumbed interfaces
# Outupt is not sorted or filtered
function up_if_list
{
    typeset uil_uname_s=${1}
    typeset uil_line=

    if [[ -z ${uil_uname_s} ]]
    then
        uil_uname_s=$(uname -s)
    fi

    case ${uil_uname_s} in
        AIX)
            ifconfig -l
            return 0
            ;;
        Linux)
            # grep -v SLAVE removes LACP leg devices
            ip link show up | grep '^[0-9][0-9]*:' | grep -v SLAVE | while read uil_line
            do
                uil_line=${uil_line#*: }
                uil_line=${uil_line%%:*}
                if [[ ${uil_line} != usb* ]]  # Filter out usbX
                then
                    echo ${uil_line}
                fi
            done
            return 0
            ;;
        SunOS)
            ifconfig -a | grep '[a-z]*:' | while read uil_line
            do
                uil_line=${uil_line%%:*}
                if [[ ${uil_line} != sppp* ]]  # Filter out XSCF
                then
                    echo ${uil_line}
                fi
            done
            return 0
            ;;
    esac

    return 1
}

# ----------------------------------------------------------------------------
# get_ip_for_if
# Determine IP address for a given if
function get_ip_for_if
{
    typeset ifi_device=${1}
    typeset ifi_uname_s=${2}
    typeset ifi_line=
    typeset ifi_ipaddr=

    if [[ -z ${ifi_device} ]]
    then
        return 1
    fi

    if [[ -z ${ifi_uname_s} ]]
    then
        ifi_uname_s=$(uname -s)
    fi

    case ${ifi_uname_s} in
        AIX|SunOS)
            ifi_line=$(ifconfig ${ifi_device} 2>/dev/null | grep inet)
            ifi_line=${ifi_line#*inet }
            ifi_line=${ifi_line%% *}
            echo ${ifi_line}
            return 0
            ;;
        Linux)
            ifi_line=$(ip -4 addr show ${ifi_device} 2>/dev/null | grep inet)
            ifi_line=${ifi_line#*inet }
            ifi_line=${ifi_line%%/*}
            echo ${ifi_line}
            return 0
            ;;
    esac

    return 1
}

# ----------------------------------------------------------------------------
# get_hn_for_if
# Determine hostname for a given if
function get_hn_for_if
{
    typeset hfi_device=${1}
    typeset hfi_uname_s=${2}
    typeset hfi_line=
    typeset hfi_ipaddr=

    if [[ -z ${hfi_uname_s} ]]
    then
        hfi_uname_s=$(uname -s)
    fi

    if ! hfi_ipaddr=$(get_ip_for_if ${hfi_device} ${hfi_uname_s})
    then
        return 1
    fi

    case ${hfi_uname_s} in
        AIX)
            if ! hfi_line=$(host ${hfi_ipaddr})
            then
                echo
                return 1
            fi
            hfi_line=${hfi_line% is*}

            echo ${hfi_line}
            return 0
            ;;
        Linux)
            if ! hfi_line=$(getent hosts ${hfi_ipaddr})
            then
                echo
                return 1
            fi

            hfi_line=${hfi_line#*[[:space:]]}   # Remove the leading IP address
            hfi_line=${hfi_line##+( )}          # Remove the leading space (the Linux way) (if any)
            hfi_line=${hfi_line%%[[:space:]]*}  # Remove any aliases
            hfi_line=${hfi_line%%.*}            # Remove trailing .bloomberg.com (if it exists)
            echo ${hfi_line}
            return 0
            ;;
        SunOS)
            if ! hfi_line=$(getent hosts ${hfi_ipaddr})
            then
                echo
                return 1
            fi

            hfi_line=${hfi_line#*[[:space:]]}   # Remove the leading IP address
            hfi_line=${hfi_line%%[[:space:]]*}  # Remove any aliases
            hfi_line=${hfi_line%%.*}            # Remove trailing .bloomberg.com (if it exists)
            echo ${hfi_line}
            return 0
            ;;
    esac

    return 1
}
