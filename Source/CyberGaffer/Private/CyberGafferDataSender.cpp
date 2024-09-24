#include "CyberGafferDataSender.h"

#include "HAL/RunnableThread.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Modules/ModuleManager.h"
#include "Misc/Timespan.h"
#include "Misc/DateTime.h"

#include "CyberGafferLog.h"


FCyberGafferDataSender::FCyberGafferDataSender() {
	CYBERGAFFERVERB_LOG(Log, TEXT("FCyberGafferDataSender::FCyberGafferDataSender"))
}

FCyberGafferDataSender::~FCyberGafferDataSender() {
	CYBERGAFFERVERB_LOG(Log, TEXT("FCyberGafferDataSender::~FCyberGafferDataSender"))
	if (_thread) {
		_thread->Kill();
		delete _thread;
	}
}

bool FCyberGafferDataSender::Init() {
#if WITH_EDITOR
	GEngine->OnEditorClose().AddSP(this, &FCyberGafferDataSender::Stop);
#endif
	return FRunnable::Init();
}

uint32 FCyberGafferDataSender::Run() {
	CYBERGAFFERVERB_LOG(Log, TEXT("FCyberGafferDataSender::Run: thread created"));

	FRequestResult requestResult;
	const auto waitForResponseMaxTime = FTimespan::FromSeconds(1);
	
	while (!_exitRequested)
	{
		FScopeLock lock(&_dataMutex);
		if (_packageToSend.Data.Num() > 0) {
			// UE_LOG(LogTemp, Log, TEXT("FCyberGafferDataSender::Run: preparing to send data to the server"));
			const auto dataToSendLength = _packageToSend.Data.Num();
			const auto requestSent = SendData();
				
			lock.Unlock();

			const auto requestSentTime = FDateTime::UtcNow();
			
			while (true) {
				if (_exitRequested) {
					return 0;
				}

				requestResult = GetRequestResult();
				if (requestResult.State == ERequestState::Ready) {
					break;
				}

				if ((FDateTime::UtcNow() - requestSentTime) >= waitForResponseMaxTime) {
					break;
				}

				FPlatformProcess::Sleep(0.025f);
			}
			
			// if (currentRequest.IsReady()) {
			// 	const auto result = currentRequest.Get();
			// 	if (result != EHttpStatusCode::OK) {
			// 		CYBERGAFFER_LOG(Warning, TEXT("FCyberGafferDataSender::Run: failed to send data, server response code: %i, stop sending for 1 second"), static_cast<int32>(result));
			// 		FPlatformProcess::Sleep(1.0f);
			// 	} else {
			// 		CYBERGAFFERVERB_LOG(Log, TEXT("FCyberGafferDataSender::Run: send %i bytes to server, server response: %i"), dataToSendLength, static_cast<int32>(result));
			// 	}
			// } else {
			// 	CYBERGAFFER_LOG(Warning, TEXT("FCyberGafferDataSender::Run: future is not ready"));
			// }

			if (requestResult.State == ERequestState::Ready) {
				if (requestResult.HttpStatus != EHttpStatusCode::OK) {
					CYBERGAFFER_LOG(Warning, TEXT("FCyberGafferDataSender::Run: failed to send data, server response code: %i, stop sending for 1 second"), static_cast<int32>(requestResult.HttpStatus));
					FPlatformProcess::Sleep(1.0f);
				} else {
					CYBERGAFFERVERB_LOG(Log, TEXT("FCyberGafferDataSender::Run: send %i bytes to server, server response: %i"), dataToSendLength, static_cast<int32>(requestResult.HttpStatus));
				}
			} else {
				_requestResult.Request->OnProcessRequestComplete().Unbind();
				_requestResult.Request->CancelRequest();
				CYBERGAFFER_LOG(Warning, TEXT("FCyberGafferDataSender::Run: future is not ready"));
			}
		} else {
			lock.Unlock();
			//CYBERGAFFERVERB_LOG(Log, TEXT("FCyberGafferDataSender::Run: nothing to send"));
		}
	}
	
	CYBERGAFFERVERB_LOG(Warning, TEXT("FCyberGafferDataSender::Run: thread is destroyed"));
	
	return 0;
}

void FCyberGafferDataSender::Stop() {
	_exitRequested = true;

	// if (_currentRequest.IsValid()) {
	// 	_currentRequest.Reset();
	// }
}

void FCyberGafferDataSender::SetPackageToSend(FCyberGafferDataPackage package) {
	CYBERGAFFERVERB_LOG(Log, TEXT("FCyberGafferDataSender::SetData: new data size: %i"), package.Data.Num());
	
	if (_exitRequested) {
		return;
	}
	
	FScopeLock lock(&_dataMutex);
	
	_packageToSend = MoveTemp(package);

	lock.Unlock(); 
	
	if (_thread == nullptr && !_exitRequested) {
		CreateThread();
	}
	
	CYBERGAFFERVERB_LOG(Log, TEXT("FCyberGafferDataSender::SetData: complete"));
}

// TFuture<EHttpStatusCode> FCyberGafferDataSender::SendData() {
bool FCyberGafferDataSender::SendData() {
	CYBERGAFFERVERB_LOG(Log, TEXT("FCyberGafferDataSender::SendData: start sending..."));

	{
		FScopeLock lock(&_requestResultMutex);
		_requestResult.State = ERequestState::Undefined;
		_requestResult.Request = nullptr;
		_requestResult.Succeeded = false;
		_requestResult.HttpStatus = EHttpStatusCode::BadRequest;
	}
	
	auto& httpModule = FModuleManager::GetModuleChecked<FHttpModule>(TEXT("HTTP"));
	const FString serverUrl = FString::Format(TEXT("http://{0}:{1}/UploadLightIdealTaskFromUnrealEngine"), {_packageToSend.ServerIpAddress, _packageToSend.ServerPort});
	
	const auto dataSize = _packageToSend.Data.Num();
	
	auto request = httpModule.CreateRequest();
	request->SetURL(serverUrl);
	request->SetVerb("POST");
	request->SetHeader("Content-Type", TEXT("application/octet-stream"));
	request->SetContent(MoveTemp(_packageToSend.Data));

	request->OnProcessRequestComplete().BindSP(this, &FCyberGafferDataSender::SetRequestResult);

	// auto resultPromise = MakeShared<TPromise<EHttpStatusCode>>();
	// request->OnProcessRequestComplete().BindLambda([resultPromise](FHttpRequestPtr request, FHttpResponsePtr response, bool succeeded)	{
	// 	if (response != nullptr) {
	// 		resultPromise->EmplaceValue(static_cast<EHttpStatusCode>(response->GetResponseCode()));
	// 	} else {
	// 		resultPromise->EmplaceValue(EHttpStatusCode::BadRequest);
	// 	}
	// });
	
	// const auto result = request->ProcessRequest();
	// _packageToSend = {};
	//
	// CYBERGAFFERVERB_LOG(Log, TEXT("FCyberGafferDataSender::SendData: sending complete, wait for server response"));
	//
	// return resultPromise->GetFuture();

	{
		FScopeLock lock(&_requestResultMutex);
		_requestResult.State = ERequestState::Processing;
		_requestResult.Request = request.ToSharedPtr();
	}
	
	return request->ProcessRequest();
}

void FCyberGafferDataSender::CreateThread() {
	FScopeLock lock(&_threadMutex);

	if (_thread != nullptr) {
		CYBERGAFFER_LOG(Error, TEXT("FCyberGafferDataSender::CreateThread: thread already initialized"));
		return;
	}
	
	CYBERGAFFERVERB_LOG(Log, TEXT("FCyberGafferDataSender::CreateThread"))
	_thread = FRunnableThread::Create(this, TEXT("CyberGafferDataSenderThread"));
}

void FCyberGafferDataSender::SetRequestResult(FHttpRequestPtr request, FHttpResponsePtr response, bool succeeded) {
	FScopeLock lock(&_requestResultMutex);

	if (_requestResult.Request != request) {
		CYBERGAFFER_LOG(Warning, TEXT("FCyberGafferDataSender::SetRequestResult: possible old request processing"));
		return;
	}

	_requestResult.State = ERequestState::Ready;
	_requestResult.Succeeded = succeeded;
	
	if (response != nullptr) {
		_requestResult.HttpStatus = static_cast<EHttpStatusCode>(response->GetResponseCode());
	} else {
		_requestResult.HttpStatus = EHttpStatusCode::BadRequest;
	}
}

FRequestResult FCyberGafferDataSender::GetRequestResult() {
	FScopeLock lock(&_requestResultMutex);
	return _requestResult;
}
