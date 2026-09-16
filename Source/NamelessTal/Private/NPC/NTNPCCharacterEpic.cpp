// Fill out your copyright notice in the Description page of Project Settings.


#include "NPC/NTNPCCharacterEpic.h"
#include "NamelessTalGameMode.h"

void ANTNPCCharacterEpic::HandleDeath()
{
	Super::HandleDeath();

	if (ANamelessTalGameMode* GameMode = Cast<ANamelessTalGameMode>(GetWorld()->GetAuthGameMode()))
	{
		GameMode->SetIsEpicKilled(true);
	}
}

