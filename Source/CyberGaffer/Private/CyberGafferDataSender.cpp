// Copyright (c) 2023 Exostartup LLC.
//
// Distributed under the terms of the MIT License (see the accompanying LICENSE.txt file or get a copy at http://opensource.org/licenses/MIT).


#include "CyberGafferDataSender.h"

#include "HAL/RunnableThread.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"

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
	return FRunnable::Init();
}

uint32 FCyberGafferDataSender::Run() {
	CYBERGAFFERVERB_LOG(Log, TEXT("FCyberGafferDataSender::Run: thread created"));
	while (!_exitRequested) {
		{
			FScopeLock lock(&_dataMutex);
			if (_packageToSend.Data.Num() > 0) {
				// UE_LOG(LogTemp, Log, TEXT("FCyberGafferDataSender::Run: preparing to send data to the server"));
				const auto dataToSendLength = _packageToSend.Data.Num();
				auto futureResult = SendData();
				
				lock.Unlock();

				// TODO: Replace Wait() with WaitFor()
				// futureResult.WaitFor(FTimespan(0, 0, 2));
				futureResult.Wait();

				if (futureResult.IsReady()) {
					const auto result = futureResult.Get();
					if (result != EHttpStatusCode::OK) {
						CYBERGAFFER_LOG(Warning, TEXT("FCyberGafferDataSender::Run: failed to send data, server response code: %i, stop sending for 1 second"), static_cast<int32>(result));
						FPlatformProcess::Sleep(1.0f);
					} else {
						CYBERGAFFERVERB_LOG(Log, TEXT("FCyberGafferDataSender::Run: send %i bytes to server, server response: %i"), dataToSendLength, static_cast<int32>(result));
					}
				} else {
					CYBERGAFFER_LOG(Warning, TEXT("FCyberGafferDataSender::Run: future is not ready"));
				}
			} else {
				CYBERGAFFERVERB_LOG(Log, TEXT("FCyberGafferDataSender::Run: nothing to send"));
			}
		}
		// TODO: added to avoid spamming the server in dev state. Should be removed in future.  
		FPlatformProcess::Sleep(1.0f);
	}
	
	CYBERGAFFERVERB_LOG(Warning, TEXT("FCyberGafferDataSender::Run: thread is destroyed"));
	
	return 0;
}

void FCyberGafferDataSender::Stop() {
	_exitRequested = true;
}

void FCyberGafferDataSender::SetPackageToSend(FCyberGafferDataPackage package) {
	CYBERGAFFERVERB_LOG(Log, TEXT("FCyberGafferDataSender::SetData: new data size: %i"), package.Data.Num());
	
	if (_exitRequested) {
		return;
	}
	
	FScopeLock lock(&_dataMutex);
	
	_packageToSend = MoveTemp(package);

	if (_thread == nullptr && !_exitRequested) {
		CreateThread();
	}
	
	CYBERGAFFERVERB_LOG(Log, TEXT("FCyberGafferDataSender::SetData: complete"));
}

TFuture<EHttpStatusCode> FCyberGafferDataSender::SendData() {
	CYBERGAFFERVERB_LOG(Log, TEXT("FCyberGafferDataSender::SendData: start sending..."));
	
	auto& httpModule = FModuleManager::GetModuleChecked<FHttpModule>(TEXT("HTTP"));
	const FString serverUrl = FString::Format(TEXT("http://{0}:{1}/UploadEnvironment"), {_packageToSend.ServerIpAddress, _packageToSend.ServerPort});
	
	const auto dataSize = _packageToSend.Data.Num();
	
	auto request = httpModule.CreateRequest();
	request->SetURL(serverUrl);
	request->SetVerb("POST");
	request->SetHeader("Content-Type", TEXT("application/octet-stream"));
	request->SetContent(MoveTemp(_packageToSend.Data));

	auto resultPromise = MakeShared<TPromise<EHttpStatusCode>>();
	request->OnProcessRequestComplete().BindLambda([resultPromise](FHttpRequestPtr request, FHttpResponsePtr response, bool succeeded)	{
		resultPromise->EmplaceValue(static_cast<EHttpStatusCode>(response->GetResponseCode()));
	});
	
	const auto result = request->ProcessRequest();
	_packageToSend = {};

	CYBERGAFFERVERB_LOG(Log, TEXT("FCyberGafferDataSender::SendData: sending complete, wait for server response"));

	return resultPromise->GetFuture();
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
