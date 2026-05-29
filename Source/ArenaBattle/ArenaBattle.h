// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#define LOG_NETMODEINFO ((GetNetMode() == ENetMode::NM_Client) ? \
	*FString::Printf(TEXT("CLIENT%d"), UE::GetPlayInEditorID()) : \
	((GetNetMode() == ENetMode::NM_Standalone) ? \
	TEXT("STANDALONE") : TEXT("SERVER")))

// __FUNCTION__ -> C문자열로 함수 이름값 전달해줌, 언리얼은 C문자열 말고 wchar_t 문자열을 쓰기 때문에 ANSI_TO_TCHAR 함수로 변환해줘야함
// __FILE__, __LINE__ 매크로도 있음. 이걸로 에러로그시스템 구체화 가능
#define LOG_CALLINFO ANSI_TO_TCHAR(__FUNCTION__)

#define AB_LOG(LogCat, Verbosity, Format, ...) \
	UE_LOG(LogCat, Verbosity, TEXT("[%s] %s %s"), \
		LOG_NETMODEINFO, LOG_CALLINFO, *FString::Printf(Format, ##__VA_ARGS__))

DECLARE_LOG_CATEGORY_EXTERN(LogABNetwork, Log, All);