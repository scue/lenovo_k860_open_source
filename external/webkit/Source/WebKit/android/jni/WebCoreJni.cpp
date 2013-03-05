/*
 * Copyright 2007, The Android Open Source Project
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#define LOG_TAG "webcoreglue"

#include "config.h"
#include "WebCoreJni.h"

#include "NotImplemented.h"
#include <JNIUtility.h>
#include <jni.h>
#include <utils/Log.h>

namespace android {

AutoJObject getRealObject(JNIEnv* env, jobject obj)
{
    jobject real = env->NewLocalRef(obj);
    LOG_ASSERT(real, "The real object has been deleted!");
    return AutoJObject(env, real);
}

/**
 * Helper method for checking java exceptions
 * @return true if an exception occurred.
 */
bool checkException(JNIEnv* env)
{
    if (env->ExceptionCheck() != 0)
    {
        LOGE("*** Uncaught exception returned from Java call!\n");
        env->ExceptionDescribe();
        return true;
    }
    return false;
}

// This method is safe to call from the ui thread and the WebCore thread.
WTF::String jstringToWtfString(JNIEnv* env, jstring str)
{
    if (!str || !env)
        return WTF::String();
    const jchar* s = env->GetStringChars(str, NULL);
    if (!s)
        return WTF::String();
    WTF::String ret(s, env->GetStringLength(str));
    env->ReleaseStringChars(str, s);
    checkException(env);
    return ret;
}

jstring wtfStringToJstring(JNIEnv* env, const WTF::String& str, bool validOnZeroLength)
{
    int length = str.length();
    return length || validOnZeroLength ? env->NewString(str.characters(), length) : 0;
}


#if USE(CHROME_NETWORK_STACK)
string16 jstringToString16(JNIEnv* env, jstring jstr)
{
    if (!jstr || !env)
        return string16();

    const char* s = env->GetStringUTFChars(jstr, 0);
    if (!s)
        return string16();
    string16 str = UTF8ToUTF16(s);
    env->ReleaseStringUTFChars(jstr, s);
    checkException(env);
    return str;
}

std::string jstringToStdString(JNIEnv* env, jstring jstr)
{
    if (!jstr || !env)
        return std::string();

    const char* s = env->GetStringUTFChars(jstr, 0);
    if (!s)
        return std::string();
    std::string str(s);
    env->ReleaseStringUTFChars(jstr, s);
    checkException(env);
    return str;
}

/* RK_ID:Browser DEP_RK_ID:NULL. AUT:makun2@lenovo.com. DATE:2012-04-23. START */
/**
*To fix bug:164689
*The following codes,from the old sichuan project.
*We all found this problem,and my original reslove method is to
*change the CheckJni.cpp in the vm.But maybe only change the webkit is  well.
*/
/** add by houjie **/
jstring gb2312StringToJstring(JNIEnv* env, const char* str)
{
	jclass strClass = env->FindClass("java/lang/String");
	jmethodID ctorID = env->GetMethodID(strClass, "<init>", "([BLjava/lang/String;)V");
	jbyteArray bytes = env->NewByteArray(strlen(str));
	env->SetByteArrayRegion(bytes,0,strlen(str), (jbyte*)str);
	jstring encoding = env->NewStringUTF("gb2312");
	return (jstring)env->NewObject(strClass, ctorID, bytes, encoding);
}

bool checkGB2312String(const std::string& str)
{
	if(str.length() < 2)
    {
	    return false;
	}

	for(int i = 0; i < str.length();)
	{
	     if(str[i] < 0)
	     {
	         unsigned char ch1;
	         unsigned char ch2;
             ch1 = (unsigned char)str[i];
             ch2 = (unsigned char)str[++i];
             if(ch1 >= 129 && ch1 <= 254 && ch2 >= 64 && ch2 <= 254)
             {
                i++;
             }
             else
             {
                return false;
             }
         }
         else
         {
            if(str[++i] == 0)
            {
                i++;
            }
        }
    }
    return true;
}


/** add end **/
/* RK_ID:Browser DEP_RK_ID:NULL. AUT:makun2@lenovo.com. DATE:2012-04-23. END */

jstring stdStringToJstring(JNIEnv* env, const std::string& str, bool validOnZeroLength)
{
/* RK_ID:Browser DEP_RK_ID:NULL. AUT:makun2@lenovo.com. DATE:2012-04-23. START */
/**
*To fix bug:164689
*The following codes,from the old sichuan project.
*We all found this problem,and my original reslove method is to
*change the CheckJni.cpp in the vm.But maybe only change the webkit is  well.
*/
	/** add by houjie **/
	//const char* ch = reinterpret_cast<const char*>(str.c_str());

    jstring result; 
	
	int len = 1024;
	char strcheck[len];
	char* bytes;
	strncpy(strcheck,str.c_str(),str.size());
	strcheck[str.size()]='\0';

	bool isUtfString = true;

	bytes = strcheck;
	while(*bytes!='\0')
	{
		char utf8=*(bytes++);
		switch (utf8>>4)
		{
			case 0x00:
			case 0x01:
			case 0x02:
			case 0x03:
			case 0x04:
			case 0x05:
			case 0x06:
			case 0x07:
			{
				//
				break;
			}
			case 0x08:
			case 0x09:
			case 0x0a:
			case 0x0b:
			case 0x0f:
			{
				isUtfString = false;
				//*(bytes-1)='?';
				break;
			}
			case 0x0e:
			{
				utf8=*(bytes++);
				if((utf8 & 0xc0)!=0x80)
				{
					isUtfString = false;
					//*(bytes-2) = '?';
					/**                         **/
					//bytes--;
					//////////*(bytes++) = 0x80;
					///////////*(bytes++) = 0x80;
				}
				break;
			}
			case 0x0c:
			case 0x0d:
			{
				utf8=*(bytes++);
				if((utf8 & 0xc0)!=0x80)
				{
					isUtfString = false;
					//*(bytes-2) = '?';
					//bytes--;
					////*(bytes++) = 0x80;
				}
				break;
			}
		}
		//
		if(!isUtfString) break;
	}

	if(checkGB2312String(str))
	{
		//gb2312ToUTF8(str.c_str(),str.length(),strcheck,len);
	    return gb2312StringToJstring(env,str.c_str());

	}
	else
	{
	bytes = strcheck;
	while(*bytes!='\0')
	{
		char utf8=*(bytes++);
		switch (utf8>>4)
		{
			case 0x00:
			case 0x01:
			case 0x02:
			case 0x03:
			case 0x04:
			case 0x05:
			case 0x06:
			case 0x07:
			{
				//
				break;
			}
			case 0x08:
			case 0x09:
			case 0x0a:
			case 0x0b:
			case 0x0f:
			{

		//		isUtfString = false;

				*(bytes-1)='?';
				break;
			}
			case 0x0e:
			{
				utf8=*(bytes++);
				if((utf8 & 0xc0)!=0x80)
				{
					
				//	isUtfString = false;
				   *(bytes-2) = '?';

					/**                         **/
					bytes--;
					//*(bytes++) = 0x80;
					//*(bytes++) = 0x80;
				}
				break;
			}
			case 0x0c:
			case 0x0d:
			{
				utf8=*(bytes++);
				if((utf8 & 0xc0)!=0x80)
				{
					//isUtfString = false;
					*(bytes-2) = '?';
					bytes--;
					//*(bytes++) = 0x80;
				}
				break;
			}
		}
		//if(!isUtfString) break;
	}
	}

	/** add end **/
/* RK_ID:Browser DEP_RK_ID:NULL. AUT:makun2@lenovo.com. DATE:2012-04-23. END */
    return !str.empty() || validOnZeroLength ? env->NewStringUTF(strcheck):0;//str.c_str()) : 0;
}


#endif

}
