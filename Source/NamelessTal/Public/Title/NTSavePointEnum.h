// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NTSavePointEnum.generated.h"

/**
 * 
 */
UENUM(BlueprintType)
enum class ENTRespawnPointEnum : uint8
{
	StartPoint = 0,
	FrontBossRoom = 10,
	DownEpicRoom = 20,
};
