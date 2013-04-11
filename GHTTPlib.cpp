#include "stdafx.h"
#include "GHTTPlib.h"
#include <windows.h>
#include <comdef.h>
#include <string>
#include <atlbase.h>
#include <atlconv.h>

using namespace std;

void SimpleArray::Init(const _variant_t& variant)
{
	array_ = variant;
	SafeArrayAccessData(array_.parray, (void**)&buffer_);
	SafeArrayGetLBound(array_.parray, 1, &lower_bound_);
	SafeArrayGetUBound(array_.parray, 1, &upper_bound_);
}

SimpleArray::SimpleArray()
	:array_(NULL)
{

}

SimpleArray::SimpleArray(const _variant_t& variant)
{
	Init(variant);
}

SimpleArray::~SimpleArray()
{
	SafeArrayUnaccessData(array_.parray);
}

SimpleArray& SimpleArray::operator =(const _variant_t& variant)
{
	Init(variant);
	return *this;
}

const char* SimpleArray::GetData() const
{
	return buffer_;
}

SimpleArray::operator const char*()
{
	return buffer_;
}

long SimpleArray::upper_bound() const
{
	return upper_bound_;
}

long SimpleArray::lower_bound() const
{
	return lower_bound_;
}
////////////////////////////////////////////////////////////////////////////
HTTPResponse::HTTPResponse(const CComPtr<IXMLHTTPRequest>& http_request_ptr)
	: http_request_ptr_(http_request_ptr)
{

}

HTTPResponse::HTTPResponse()
	:http_request_ptr_(NULL)
{

}

HTTPResponse& HTTPResponse::operator =(const CComPtr<IXMLHTTPRequest>& http_request_ptr)
{
	this->http_request_ptr_ = http_request_ptr;
	return *this;
}

string HTTPResponse::GetText()
{
	BSTR response_text = SysAllocString(NULL);
	int	 response_length;
	string result;

	http_request_ptr_->get_responseText(&response_text);
	response_length = wcslen(response_text);

	// unicode转ANSI
	int ansi_char_count = WideCharToMultiByte(CP_ACP, 
		0, 
		response_text, 
		response_length, 
		NULL,
		0,
		NULL,
		NULL);

	char* ansi_string = new char[ansi_char_count];
	memset(ansi_string, 0, ansi_char_count);
	WideCharToMultiByte(CP_ACP, 0, response_text, response_length, ansi_string, ansi_char_count, NULL, NULL);
	result = ansi_string;
	delete[] ansi_string;

	SysFreeString(response_text);
	return result;
}

int HTTPResponse::GetStatusCode()
{
	long status;
	http_request_ptr_->get_status(&status);
	return status;
}

string HTTPResponse::GetHeader(const char* header_name)
{
	USES_CONVERSION;

	_bstr_t result = http_request_ptr_->getResponseHeader(A2W(header_name));

	return W2A(result);
}

long HTTPResponse::GetLength()
{
	string length_string = GetHeader("Content-Length");
	return atoi(length_string.c_str());
}

void HTTPResponse::GetData(SimpleArray& output_data)
{
	_variant_t data;
	http_request_ptr_->get_responseBody(&data);
	output_data = data;
}

////////////////////////////////////////////////////////////////////////////////////////
HTTPRequest::HTTPRequest(void)
	: http_request_ptr_(NULL)
{
	CoInitialize(NULL);

	HRESULT hr;
	CLSID class_id;

	hr = CLSIDFromProgID(_bstr_t("MSXML2.XMLHTTP"), &class_id);

	if (FAILED(hr))
	{
		throw("unable to initialize MSXML2.XMLHTTP!");
	}

	hr = CoCreateInstance(class_id, NULL, CLSCTX_ALL, IID_IXMLHttpRequest, (void**)&http_request_ptr_);
	if (FAILED(hr))
	{
		throw("unable to get IXMLHttpRequest interface.");
	}
}


HTTPRequest::~HTTPRequest(void)
{
	http_request_ptr_.Release();
	CoUninitialize();
}

void HTTPRequest::SetRequestHeader(const char* header, const char* value)
{
	try
	{
		http_request_ptr_->setRequestHeader(_bstr_t(header), _bstr_t(value));
	}
	catch (_com_error& e)
	{
		_bstr_t info = e.Description();
		_asm nop;
	}
	
}

void HTTPRequest::SetRequestHeader(const Headers& header)
{
	this->headers_ = header;
}

HTTPResponse HTTPRequest::SendRequest(const char* method, const char* url, const char* post_data)
{
	_bstr_t unicode_url;
	USES_CONVERSION;

	unicode_url = A2W(url);

	http_request_ptr_->open(_bstr_t(method), 
		unicode_url, 
		_variant_t(false), 
		_variant_t(NULL), 
		_variant_t(NULL));

	for (Headers::const_iterator it = headers_.begin(); it != headers_.end(); it++)
	{
		SetRequestHeader(it->first.c_str(), it->second.c_str());
	}

	try{
		http_request_ptr_->send(_bstr_t(post_data));

		long ready_state;

		while (1)
		{
			http_request_ptr_->get_readyState(&ready_state);
			if (ready_state == 4)
				break;
		}
	}
	catch (_com_error&)
	{

	}

	HTTPResponse response(http_request_ptr_);
	return response;
}

HTTPResponse HTTPRequest::Get(const char* url)
{
	return SendRequest("GET", url, NULL);
}

int HTTPRequest::Get(const char* url, string& output_result)
{
	HTTPResponse response = Get(url);
	output_result = response.GetText();

	return response.GetStatusCode();
}

HTTPResponse HTTPRequest::Post(const char* url, const char* post_data)
{
	return SendRequest("POST", url, post_data);
}

int HTTPRequest::Post(const char* url, const char* post_data, string& output_result)
{
	HTTPResponse response = Post(url, post_data);
	output_result = response.GetText();
	return response.GetStatusCode();
}