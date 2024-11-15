#pragma once

#include "CoreMinimal.h"
#include "HttpModule.h"
#include "Async/Future.h"
#include "HAL/Runnable.h"
#include "Templates/SharedPointer.h"

#include "CyberGafferDataPackage.h"

enum class EHttpStatusCode : int32 {
    // Informational 1xx
    Continue = 100, // The server has received the request headers and the client should proceed to send the request body.
    SwitchingProtocols = 101, // The requester has asked the server to switch protocols and the server has agreed to do so.
    Processing = 102, // The server is currently processing the request and will return the final response later.
    EarlyHints = 103, // Used to return some response headers before the final response.

    // Successful 2xx
    OK = 200, // The request has succeeded, and the information returned with the response is dependent on the method used in the request.
    Created = 201, // The request has been fulfilled, and a new resource has been created as a result.
    Accepted = 202, // The request has been accepted for processing, but the processing has not been completed.
    NonAuthoritativeInformation = 203, // The server is a transforming proxy and received a 200 OK response from the origin, but is returning a modified version.
    NoContent = 204, // The server successfully processed the request but is not returning any content.
    ResetContent = 205, // The server successfully processed the request, and there is no additional content to send in the response.
    PartialContent = 206, // The server is delivering only part of the resource due to a range header sent by the client.
    MultiStatus = 207, // The message body that follows is by default an XML message and can contain a number of separate response codes, depending on how many sub-requests were made.
    AlreadyReported = 208, // The members of a DAV binding have already been enumerated in a preceding part of the (multistatus) response, and are not being included again.
    IMUsed = 226, // The server has fulfilled a request for the resource, and the response is a representation of the result of one or more instance-manipulations applied to the current instance.

    // Redirection 3xx
    MultipleChoices = 300, // The response represents multiple choices and the client can select a resource by redirecting to it.
    MovedPermanently = 301, // The requested resource has been assigned a new permanent URI and any future references to this resource should use one of the returned URIs.
    Found = 302, // The requested resource has been found under a different URI, and the client should redirect to that URI.
    SeeOther = 303, // The response can be found under a different URI, and should be retrieved using a GET method on that resource.
    NotModified = 304, // The server tells the client that the requested resource has not been modified.

    // Client Errors 4xx
    BadRequest = 400, // The server cannot process the request due to something that is perceived to be a client error (e.g., malformed request syntax).
    Unauthorized = 401, // The client must authenticate to get the requested response.
    PaymentRequired = 402, // Reserved for future use.
    Forbidden = 403, // The server understands the request, but it won't fulfill it unless it's authorized.
    NotFound = 404, // The server has not found anything matching the Request-URI.
    MethodNotAllowed = 405, // The method specified in the request is not allowed for the resource identified by the Request-URI.
    NotAcceptable = 406, // The resource identified by the request is only capable of generating response entities that have content characteristics not acceptable according to the accept headers sent in the request.
    ProxyAuthenticationRequired = 407, // The client must first authenticate itself with the proxy.
    RequestTimeout = 408, // The client did not produce a request within the time that the server was prepared to wait.
    Conflict = 409, // The request could not be completed due to a conflict with the current state of the target resource.
    Gone = 410, // The requested resource is no longer available at the server, and no forwarding address is known.
    LengthRequired = 411, // The server refuses to accept the request without a defined Content-Length.
    PreconditionFailed = 412, // The precondition given in one or more of the request-header fields evaluated to false when it was tested on the server.
    PayloadTooLarge = 413, // The server is refusing to process a request because the request payload is larger than the server is willing or able to process.
    URITooLong = 414, // The server is refusing to service the request because the request-target is longer than the server is willing to interpret.
    UnsupportedMediaType = 415, // The server is refusing to service the request because the entity of the request is in a format not supported by the requested resource for the requested method.
    RangeNotSatisfiable = 416, // A server should return a response with this status code if a request included a Range request-header field, and none of the range-specifier values in this field overlap the current extent of the selected resource.
    ExpectationFailed = 417, // The expectation given in an Expect request-header field could not be met by this server.
    ImATeapot = 418, // This code was defined in 1998 as one of the traditional IETF April Fools' jokes, in RFC 2324, Hyper Text Coffee Pot Control Protocol.
    MisdirectedRequest = 421, // The request was directed at a server that is not able to produce a response.
    UnprocessableEntity = 422, // The request was well-formed but was unable to be followed due to semantic errors.
    Locked = 423, // The resource that is being accessed is locked.
    FailedDependency = 424, // The request failed due to failure of a previous request.
    UpgradeRequired = 426, // The client should switch to a different protocol.
    PreconditionRequired = 428, // The origin server requires the request to be conditional.
    TooManyRequests = 429, // The user has sent too many requests in a given amount of time.
    RequestHeaderFieldsTooLarge = 431, // The server is unwilling to process the request because either an individual header field, or all the header fields collectively, are too large.
    UnavailableForLegalReasons = 451, // A server operator has received a legal demand to deny access to a resource or to a set of resources that includes the requested resource.

    // Server Errors 5xx
    InternalServerError = 500, // The server has encountered a situation it doesn't know how to handle.
    NotImplemented = 501, // The server is not capable of performing the request.
    BadGateway = 502, // The server, while acting as a gateway or proxy, received an invalid response from the upstream server it accessed in attempting to fulfill the request.
    ServiceUnavailable = 503, // The server is currently unable to handle the request due to temporary overloading or maintenance of the server.
    GatewayTimeout = 504, // The server, while acting as a gateway or proxy, did not receive a timely response from the upstream server or application.
	HTTPVersionNotSupported = 505, // The server does not support the HTTP protocol version that was used in the request.
	VariantAlsoNegotiates = 506, // Transparent content negotiation for the request results in a circular reference.
	InsufficientStorage = 507, // The server is unable to store the representation needed to complete the request.
	LoopDetected = 508, // The server detected an infinite loop while processing the request.
	NotExtended = 510, // Further extensions to the request are required for the server to fulfill it.
	NetworkAuthenticationRequired = 511, // The client needs to authenticate to gain network access.

	// Unofficial Codes
	Checkpoint = 103, // Used in the resumable requests proposal to resume aborted PUT or POST requests.
	TLSHandshake = 525, // An error occurred during a TLS handshake.
};

enum class ERequestState : int32 {
	Undefined,
	Processing,
	Ready
};

struct FRequestResult {
public:
	ERequestState State = ERequestState::Undefined;
	TSharedPtr<IHttpRequest> Request = nullptr;
	bool Succeeded = false;
	EHttpStatusCode HttpStatus = EHttpStatusCode::BadRequest;
};

class FCyberGafferDataSender : public FRunnable, public TSharedFromThis<FCyberGafferDataSender, ESPMode::ThreadSafe> {
public:
	FCyberGafferDataSender();
	virtual ~FCyberGafferDataSender() override;

	// FRunnable interface
	bool Init() override;
	uint32 Run() override;
	void Stop() override;

	
	void SetPackageToSend(FCyberGafferDataPackage package);

private:
    std::atomic_bool _exitRequested = false;
	FRunnableThread* _thread = nullptr;

	FCriticalSection _dataMutex;
	FCriticalSection _threadMutex;
	FCyberGafferDataPackage _packageToSend;
	
	bool SendData();

	TFuture<EHttpStatusCode> _currentRequest = {};

	FCriticalSection _requestResultMutex;
	FRequestResult _requestResult = {};

	void CreateThread();

	void SetRequestResult(FHttpRequestPtr request, FHttpResponsePtr response, bool succeeded);
	FRequestResult GetRequestResult();
};
