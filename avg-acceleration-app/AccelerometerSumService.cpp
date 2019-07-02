#include "AccelerometerSumService.h"
#include "app-resources/resources.h"
#include "common/core/debug.h"
#include "meas_acc/resources.h"
#include "ui_ind/resources.h"
#include "whiteboard/builtinTypes/UnknownStructure.h"

#include <float.h>
#include <math.h>

// CONFIGURABLE VALUES START
// ----------------------------------------------------------------------

// The desired value publish interval in milliseconds
#define PUBLISH_INTERVAL_MS 1000

// The rate with which the accelerometer is operating. Can be one of
// 13, 26, 52, 104, 208, 416, 833, 1666
#define ACC_SAMPLE_RATE 13

// ----------------------------------------------------------------------
// CONFIGURABLE VALUES END

static_assert(
    ACC_SAMPLE_RATE == 13 || ACC_SAMPLE_RATE == 26 || ACC_SAMPLE_RATE == 52 ||
    ACC_SAMPLE_RATE == 104 || ACC_SAMPLE_RATE == 208 || ACC_SAMPLE_RATE == 416 ||
    ACC_SAMPLE_RATE == 833 || ACC_SAMPLE_RATE == 1666,
    "Selected accelerometer sample rate not supported"
);

// Some preprocessor magic for concatenating strings and numbers
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

#define ACCELEROMETER_RESOURCE_ENDPOINT "Meas/Acc/" STR(ACC_SAMPLE_RATE)

const char* const AccelerometerSumService::LAUNCHABLE_NAME = "AccSum";
static const whiteboard::ExecutionContextId sExecutionContextId =
    WB_RES::LOCAL::EXERCISE_SUMVECTOR_MIKKO::EXECUTION_CONTEXT;

static const whiteboard::LocalResourceId sProviderResources[] = {
    WB_RES::LOCAL::EXERCISE_SUMVECTOR_MIKKO::LID,
};

AccelerometerSumService::AccelerometerSumService()
    : ResourceClient(WBDEBUG_NAME(__FUNCTION__), sExecutionContextId),
      ResourceProvider(WBDEBUG_NAME(__FUNCTION__), sExecutionContextId),
      LaunchableModule(LAUNCHABLE_NAME, sExecutionContextId),
      isRunning(false)
{
}

AccelerometerSumService::~AccelerometerSumService()
{
}

bool AccelerometerSumService::initModule()
{
    if (registerProviderResources(sProviderResources) != whiteboard::HTTP_CODE_OK)
    {
        return false;
    }

    mModuleState = WB_RES::ModuleStateValues::INITIALIZED;
    return true;
}

void AccelerometerSumService::deinitModule()
{
    unregisterProviderResources(sProviderResources);
    mModuleState = WB_RES::ModuleStateValues::UNINITIALIZED;
}

/** @see whiteboard::ILaunchableModule::startModule */
bool AccelerometerSumService::startModule()
{
    mModuleState = WB_RES::ModuleStateValues::STARTED;

    return true;
}

void AccelerometerSumService::onUnsubscribeResult(whiteboard::RequestId requestId,
                                                     whiteboard::ResourceId resourceId,
                                                     whiteboard::Result resultCode,
                                                     const whiteboard::Value& rResultData)
{
    DEBUGLOG("AccelerometerSumService::onUnsubscribeResult() called.");
}

void AccelerometerSumService::onSubscribeResult(whiteboard::RequestId requestId,
                                                   whiteboard::ResourceId resourceId,
                                                   whiteboard::Result resultCode,
                                                   const whiteboard::Value& rResultData)
{
    DEBUGLOG("AccelerometerSumService::onSubscribeResult() called. resourceId: %u, result: %d", resourceId.localResourceId, (uint32_t)resultCode);

    whiteboard::Request relatedIncomingRequest;
    bool relatedRequestFound = mOngoingRequests.get(requestId, relatedIncomingRequest);

    if (relatedRequestFound)
    {
        returnResult(relatedIncomingRequest, wb::HTTP_CODE_OK);
    }
}

whiteboard::Result AccelerometerSumService::startRunning(whiteboard::RequestId& remoteRequestId)
{
    if (isRunning)
    {
        return whiteboard::HTTP_CODE_OK;
    }

    DEBUGLOG("AccelerometerSumService::startRunning()");

    // Reset max acceleration members
    this->accValue = whiteboard::FloatVector3D();
    this->mSamplesIncluded = 0;

    wb::Result result = getResource(ACCELEROMETER_RESOURCE_ENDPOINT, mMeasAccResourceId);
    if (!wb::RETURN_OKC(result))
    {
        return result;
    }

    result = asyncSubscribe(mMeasAccResourceId, AsyncRequestOptions(&remoteRequestId, 0, true));

    if (!wb::RETURN_OKC(result))
    {
        DEBUGLOG("asyncSubscribe threw error: %u", result);
        return whiteboard::HTTP_CODE_BAD_REQUEST;
    }
    isRunning = true;

    return whiteboard::HTTP_CODE_OK;
}

whiteboard::Result AccelerometerSumService::stopRunning()
{
    if (!isRunning)
    {
        return whiteboard::HTTP_CODE_OK;
    }

    if (isResourceSubscribed(WB_RES::LOCAL::EXERCISE_SUMVECTOR_MIKKO::ID) == wb::HTTP_CODE_OK)
    {
        DEBUGLOG("AccelerometerSumService::stopRunning() skipping. Subscribers still exist.");
        return whiteboard::HTTP_CODE_OK;
    }

    DEBUGLOG("AccelerometerSumService::stopRunning()");

    // Unsubscribe the LinearAcceleration resource, when unsubscribe is done, we get callback
    wb::Result result = asyncUnsubscribe(mMeasAccResourceId, NULL);
    if (!wb::RETURN_OKC(result))
    {
        DEBUGLOG("asyncUnsubscribe threw error: %u", result);
    }
    isRunning = false;
    releaseResource(mMeasAccResourceId);

    return whiteboard::HTTP_CODE_OK;
}

// This callback is called when the resource we have subscribed notifies us
void AccelerometerSumService::onNotify(whiteboard::ResourceId resourceId, const whiteboard::Value& value,
                                          const whiteboard::ParameterList& parameters)
{
    DEBUGLOG("onNotify() called.");

    // Confirm that it is the correct resource
    if (resourceId.localResourceId == WB_RES::LOCAL::MEAS_ACC_SAMPLERATE::LID)
    {
        const WB_RES::AccData& linearAccelerationValue = value.convertTo<const WB_RES::AccData&>();

        const whiteboard::Array<whiteboard::FloatVector3D>& arrayData = linearAccelerationValue.arrayAcc;

        for (const auto& sample : arrayData)
        {
            this->accValue += sample;
        }

        this->mSamplesIncluded += arrayData.size();


        uint32_t relativeTime = linearAccelerationValue.timestamp;

        // Publish a new value if it's time. Take variable overflow into account
        if (relativeTime > this->previousPublishTimestamp + PUBLISH_INTERVAL_MS || relativeTime < this->previousPublishTimestamp)
        {
            // Reset counter and notify our subscribers
            WB_RES::SampleDataValue sampleDataValue;
            sampleDataValue.relativeTime = relativeTime;
            sampleDataValue.xValue = this->accValue.x / this->mSamplesIncluded;
            sampleDataValue.yValue = this->accValue.y / this->mSamplesIncluded;
            sampleDataValue.zValue = this->accValue.z / this->mSamplesIncluded;

            // Reset members
            this->mSamplesIncluded = 0;
            this->accValue = whiteboard::FloatVector3D();
            this->previousPublishTimestamp = relativeTime;

            // and update our WB resource. This causes notification to be fired to our subscribers
            updateResource(WB_RES::LOCAL::EXERCISE_SUMVECTOR_MIKKO(),
                           ResponseOptions::Empty, sampleDataValue);

            // Blink the LED briefly to indicate a new value
            const uint16_t indicationType = 2; // SHORT_VISUAL_INDICATION
            asyncPut(WB_RES::LOCAL::UI_IND_VISUAL::ID, AsyncRequestOptions::Empty, indicationType);
        }
    }
}

void AccelerometerSumService::onSubscribe(const whiteboard::Request& request,
                                             const whiteboard::ParameterList& parameters)
{
    switch (request.getResourceConstId())
    {
    case WB_RES::LOCAL::EXERCISE_SUMVECTOR_MIKKO::ID:
    {
        // Someone subscribed to our service. Start collecting data and notifying when our service changes state (every 10 seconds)
        whiteboard::RequestId remoteRequestId;
        whiteboard::Result result = startRunning(remoteRequestId);

        if (isRunning)
        {
            return returnResult(request, whiteboard::HTTP_CODE_OK);
        }

        if (!whiteboard::RETURN_OK(result))
        {
            return returnResult(request, result);
        }
        bool queueResult = mOngoingRequests.put(remoteRequestId, request);
        (void)queueResult;
        WB_ASSERT(queueResult);
        break;
    }
    default:
        ASSERT(0); // Should not happen
    }
}

void AccelerometerSumService::onUnsubscribe(const whiteboard::Request& request,
                                               const whiteboard::ParameterList& parameters)
{
    switch (request.getResourceConstId())
    {
    case WB_RES::LOCAL::EXERCISE_SUMVECTOR_MIKKO::ID:
        stopRunning();
        returnResult(request, wb::HTTP_CODE_OK);
        break;

    default:
        returnResult(request, wb::HTTP_CODE_BAD_REQUEST);
        ASSERT(0); // Should not happen
    }
}

void AccelerometerSumService::onRemoteWhiteboardDisconnected(whiteboard::WhiteboardId whiteboardId)
{
    DEBUGLOG("AccelerometerSumService::onRemoteWhiteboardDisconnected()");
    stopRunning();
}

void AccelerometerSumService::onClientUnavailable(whiteboard::ClientId clientId)
{
    DEBUGLOG("AccelerometerSumService::onClientUnavailable()");
    stopRunning();
}