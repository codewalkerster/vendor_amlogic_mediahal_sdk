#!/system/bin/sh
#set -x

#mediahalpatch=/vendor/lib/libmediahal_tsplayer.so
mediahalPath=/system_ext/lib/libmediahal_tsplayer.system.so
omxPath=/vendor/lib/libOmxCore.so
amnuplayerPath=/system_ext/lib/libamnuplayer.so


widevinePath=/vendor/lib/liboemcrypto.so
playreadyPath=/vendor/lib/libplayready.so
secmemPath=/vendor/lib/libsecmem.so

MODULE=$1

function get_mediahl_feature() {
    echo "--->get mediahl features<---"
    MEDIAHAL_VERSION=$(grep -a "MM-module-name" ${mediahalPath})
    echo "${MEDIAHAL_VERSION}"
    echo " "
}

function get_omx_feature() {
    echo "--->get omx features<---"
    OMX_VERSION=$(grep -a "MM-module-name" ${omxPath})
    echo "${OMX_VERSION}"
    echo " "
}

function get_amnuplayer_feature() {
    echo "--->get amnuplayer features<---"
    AMNUPLAY_VERSION=$(grep -a "MM-module-name" ${amnuplayerPath})
    echo "${AMNUPLAY_VERSION}"
    echo " "
}

function get_decoder_feature() {
    echo "--->get decoder features<---"

    if [ ! -f "/sys/class/amstream/vcodec_feature" ];then
        echo "No vcodec_feature"
        return
    fi

    while :
    do
        feature=$(cat /sys/class/amstream/vcodec_feature)
        usleep 20;
        last=$(echo "${feature}" | tail -1)
        echo "${feature}"
        if [ "${last}" == "}" ]; then
            #echo end
            break;
        #else
        #   echo "-->${last}"
        fi
    done;
    echo " "
}

function get_drm_feature() {
    echo "--->get widevine features<---"
    WIDEVINE_VERSION=$(grep -a "MM-module-name" ${widevinePath})
    echo "${WIDEVINE_VERSION}"
    echo " "
    echo "--->get playready features<---"
    PLAYREADY_VERSION=$(grep -a "MM-module-name" ${playreadyPath})
    echo "${PLAYREADY_VERSION}"
    echo " "
    echo "--->get secmem features<---"
    SECMEM_VERSION=$(grep -a "MM-module-name" ${secmemPath})
    echo "${SECMEM_VERSION}"
    echo " "
}


if [ "${MODULE}" == "mediahal" ] ; then
    get_mediahl_feature
elif [ "${MODULE}" == "omx" ] ; then
    get_omx_feature
elif [ "${MODULE}" == "amnuplayer" ] ; then
    get_amnuplayer_feature
elif [ "${MODULE}" == "drm" ] ; then
    get_drm_feature
elif [ "${MODULE}" == "decoder" ] ; then
    get_decoder_feature
else
    get_mediahl_feature
    get_omx_feature
    get_amnuplayer_feature
    get_drm_feature
    get_decoder_feature
#else
#    echo Please enter the name of the module
#    echo module:mediahal,omx,amnuplayer,decoder,all
#    echo E.g:source getmmversion all
fi