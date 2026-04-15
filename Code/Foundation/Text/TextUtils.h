/*
This code is part of Aperture UI - A HTML/CSS/JS UI Middleware

Copyright (c) 2020-2025 WD Studios Corp. and/or its licensors. All rights reserved in all
media. The coded instructions, statements, computer programs, and/or related material
(collectively the "Data") in these files contain confidential and unpublished information
proprietary WD Studios and/or its licensors, which is protected by United States of
America federal copyright law and by international treaties.

This software or source code is supplied under the terms of a license
agreement and nondisclosure agreement with WD Studios Corp. and may
not be copied, disclosed, or exploited except in accordance with the
terms of that agreement. The Data may not be disclosed or distributed to
third parties, in whole or in part, without the prior written consent of
WD Studios Corp..

WD STUDIOS MAKES NO REPRESENTATION ABOUT THE SUITABILITY OF THIS
SOURCE CODE FOR ANY PURPOSE. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT
HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER, ITS AFFILIATES,
PARENT COMPANIES, LICENSORS, SUPPLIERS, OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
ANY WAY OUT OF THE USE OR PERFORMANCE OF THIS SOFTWARE OR SOURCE CODE,
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#pragma once

// Compatibility header - Text utilities have been relocated/refactored
// This header provides forward compatibility for existing code that expects Foundation/Text/TextUtils.h

#include <Foundation/Strings/String.h>
#include <Foundation/Strings/StringBuilder.h>
#include <Foundation/Strings/StringView.h>
#include <Foundation/Strings/StringUtils.h>

namespace aperture {
namespace text {

// Basic text utilities for compatibility
class TextUtils
{
public:
    // String manipulation utilities
    static nsString TrimWhitespace(const nsString& input)
    {
        nsString result = input;
        result.Trim();
        return result;
    }
    
    static nsString ToLowerCase(const nsString& input)
    {
        nsString result = input;
        result.ToLower();
        return result;
    }
    
    static nsString ToUpperCase(const nsString& input)
    {
        nsString result = input;
        result.ToUpper();
        return result;
    }
    
    static bool IsEmpty(const nsString& input)
    {
        return input.IsEmpty();
    }
    
    static bool Contains(const nsString& input, const nsString& search)
    {
        return input.FindSubString(search) != nullptr;
    }
};

} // namespace text
} // namespace aperture