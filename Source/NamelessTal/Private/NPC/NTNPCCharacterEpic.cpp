// Fill out your copyright notice in the Description page of Project Settings.


#include "NPC/NTNPCCharacterEpic.h"
#include "NamelessTalGameMode.h"
#include "Stat/StatComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

ANTNPCCharacterEpic::ANTNPCCharacterEpic()
{
	NPCGrade = ENPCGrade::Elite;
}

float ANTNPCCharacterEpic::ApplyDamage(AActor* DamageCauser, const FNPCApplyDamageData InDamageData)
{
	const float ResultDamage = Super::ApplyDamage(DamageCauser, InDamageData);

	// HP 50% 이하로 내려가면 상의/하의/어깨장식에 핏자국 노출 (Normal과 동일한 조건)
	if (!InDamageData.bIsParry && StatComponent && StatComponent->GetMaxHP() > 0.0f)
	{
		const float HPRatio = StatComponent->GetCurrentHP() / StatComponent->GetMaxHP();
		if (HPRatio <= 0.5f)
		{
			static const FName BloodAmountParam(TEXT("BloodAmount"));

			if (TopDynamicMaterial)
			{
				TopDynamicMaterial->SetScalarParameterValue(BloodAmountParam, 1.0f);
			}
			if (PantsDynamicMaterial)
			{
				PantsDynamicMaterial->SetScalarParameterValue(BloodAmountParam, 1.0f);
			}
			if (ShoulderDynamicMaterial)
			{
				ShoulderDynamicMaterial->SetScalarParameterValue(BloodAmountParam, 1.0f);
			}
		}
	}

	return ResultDamage;
}

void ANTNPCCharacterEpic::HandleDeath()
{
	Super::HandleDeath();

	if (ANamelessTalGameMode* GameMode = Cast<ANamelessTalGameMode>(GetWorld()->GetAuthGameMode()))
	{
		GameMode->SetIsEpicKilled(true);
	}
}

