#pragma once
#import "libid:F5078F18-C551-11D3-89B9-0000F81FE221" version("3.0")

#include <string>
#include <hash_map>
#include <atlbase.h>

using std::string;
using stdext::hash_map;
using namespace MSXML2;

typedef hash_map<string, string> Headers;

class SimpleArray
{
protected:
	_variant_t array_;
	long lower_bound_;
	long upper_bound_;
	const char* buffer_;
	void Init(const _variant_t& variant);
public:
	SimpleArray();
	~SimpleArray();

	operator const char*();
	SimpleArray(const _variant_t& variant);
	SimpleArray& operator=(const _variant_t& variant);
	const char* GetData() const;
	long lower_bound() const;
	long upper_bound() const;
};

class HTTPResponse
{
protected:
	CComPtr<IXMLHTTPRequest> http_request_ptr_;
public:
	string		GetText();
	string		GetHeader(const char* header_name);
	long		GetLength();
	void 		GetData(SimpleArray& output_data);
	int			GetStatusCode();
	
	HTTPResponse();
	HTTPResponse(const CComPtr<IXMLHTTPRequest>& http_request_ptr);
	HTTPResponse& operator=(const CComPtr<IXMLHTTPRequest>& http_request_ptr);
};

class HTTPRequest
{
protected:
	CComPtr<IXMLHTTPRequest> http_request_ptr_;
	void		 SetRequestHeader(const char* header, const char* value);
	HTTPResponse SendRequest(const char* method, const char* url, const char* post_data);
	Headers headers_;
public:
	HTTPRequest(void);
	~HTTPRequest(void);

	/**
	 * Get请求
	 * @param url [in] 请求的URL地址
	 * @param output_result [out] 服务器返回的信息
	 * @return HTTP状态码
	 */
	int Get(const char* url, string& output_result);
	HTTPResponse Get(const char* url);
	/**
	 * Post请求
	 * @param url [in] 请求的URL地址
	 * @param output_result 服务器返回的信息
	 * @param post_data [in] POST提交的数据
	 * @return HTTP状态码
	 */
	int Post(const char* url, const char* post_data, string& output_result);
	HTTPResponse Post(const char* url, const char* post_data);
	/**
	 * 设置HTTP请求的header
	 * @param header [in] 请求头
	 */
	void SetRequestHeader(const Headers& header);
	
};

