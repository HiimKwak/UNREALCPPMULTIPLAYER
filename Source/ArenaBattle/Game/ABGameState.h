// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "ABGameState.generated.h"

/**
 * 
 */
UCLASS()
class ARENABATTLE_API AABGameState : public AGameStateBase
{
	GENERATED_BODY()
	
	// GameMode의 StartPlay 함수에서 호출하는 함수
	virtual void HandleBeginPlay() override;
	// bReplicatedHasBegunPlay 값이 변경되면 서버/클라이언트 모두에서 실행되는 함수
	virtual void OnRep_ReplicatedHasBegunPlay() override;
};
