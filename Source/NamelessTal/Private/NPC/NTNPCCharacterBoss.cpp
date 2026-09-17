// Fill out your copyright notice in the Description page of Project Settings.


#include "NPC/NTNPCCharacterBoss.h"
#include "MCLOG.h"
#include "Game/NTGameDataSingleton.h"
#include "Stat/StatComponent.h"
#include "NamelessTalGameMode.h"

//AI
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "NPC/Components/NTLockOnTargetComponent.h"
#include "NPC/NTAIParams.h"

//AOE
#include "NPC/Boss/NTOkJangPan.h"
#include "Components/CapsuleComponent.h"

//Material
#include "Materials/MaterialInstanceDynamic.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"


ANTNPCCharacterBoss::ANTNPCCharacterBoss()
{
	NPCGrade = ENPCGrade::Boss;

	PatternsNames.Add(EBossPhase::First, TEXT(""));
	PatternsNames.Add(EBossPhase::Second, TEXT(""));
	PatternsNames.Add(EBossPhase::Third, TEXT(""));

	PhasePatternDatas.Add(EBossPhase::First, FNPCAttackPatternList());
	PhasePatternDatas.Add(EBossPhase::Second, FNPCAttackPatternList());
	PhasePatternDatas.Add(EBossPhase::Third, FNPCAttackPatternList());

	PhaseHP.Add(EBossPhase::First, 260);
	PhaseHP.Add(EBossPhase::Second, 195);
	PhaseHP.Add(EBossPhase::Third, 195);

	PhaseHeadringRotationSpeed.Add(EBossPhase::First, 90.0f);
	PhaseHeadringRotationSpeed.Add(EBossPhase::Second, 180.0f);
	PhaseHeadringRotationSpeed.Add(EBossPhase::Third, 360.0f);

	HeadringPivotComponent = CreateDefaultSubobject<USceneComponent>(TEXT("HeadringPivot"));
	HeadringPivotComponent->SetupAttachment(GetMesh(), TEXT("BossRing"));

	// Boss_Headring/Boss_HeadringHolder 에셋 자체의 피봇이 메시 중심이 아니라 캐릭터 발밑 원점에 있어서
	// (에셋의 로컬 바운즈 중심을 구해 반대로 밀어) 시각적 중심을 부착 지점으로 옮겨줌
	HeadringMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HeadringMesh"));
	HeadringMesh->SetupAttachment(HeadringPivotComponent);
	HeadringMesh->SetRelativeLocation(FVector(-0.646f, 22.360f, -169.215f));

	HeadringHolderMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HeadringHolderMesh"));
	HeadringHolderMesh->SetupAttachment(GetMesh(), TEXT("BossRing"));
	HeadringHolderMesh->SetRelativeLocation(FVector(-0.886f, 22.937f, -167.820f));
}

void ANTNPCCharacterBoss::BeginPlay()
{
	Super::BeginPlay();

	//CurrentPhase = EBossPhase::Third;

	if (ANamelessTalGameMode* GameMode = Cast<ANamelessTalGameMode>(GetWorld()->GetAuthGameMode()))
	{
		GameMode->OnCutsceneSequenceEnded.AddDynamic(this, &ANTNPCCharacterBoss::StartNextPhase);
	}

	StatComponent->MaxHP = PhaseHP[CurrentPhase];
	StatComponent->RestoreFull();
	StatComponent->SetSoulRegenBlocked(true);

	HeadringRotationSpeed = PhaseHeadringRotationSpeed[CurrentPhase];

	// 보스 공용 파츠(Top/Pants/Shoulder)는 기본값이 이미 더러운 상태(Dirt=1,Cutout=1)라
	// 스폰 시 깨끗한 상태로 시작하도록 명시적으로 0으로 덮어써야 함
	SetPartsScalarParameter(TEXT("DirtAmount"), 0.0f);
	SetPartsScalarParameter(TEXT("CutoutAmount"), 0.0f);
}

void ANTNPCCharacterBoss::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (HeadringPivotComponent)
	{
		HeadringPivotComponent->AddLocalRotation(FRotator(HeadringRotationSpeed * DeltaTime, 0.0f, 0.0f));
	}
}

void ANTNPCCharacterBoss::SetPartsScalarParameter(FName ParamName, float Value)
{
	if (TopDynamicMaterial)
	{
		TopDynamicMaterial->SetScalarParameterValue(ParamName, Value);
	}
	if (PantsDynamicMaterial)
	{
		PantsDynamicMaterial->SetScalarParameterValue(ParamName, Value);
	}
	if (ShoulderDynamicMaterial)
	{
		ShoulderDynamicMaterial->SetScalarParameterValue(ParamName, Value);
	}
}

void ANTNPCCharacterBoss::StartBossSequence(UAnimMontage* TargetMontage, bool IsProperlyEnded)
{
	MC_LOG(LogMC, Log, TEXT("Begin"));

	switch (CurrentPhase)
	{
	case EBossPhase::First:
		CurrentPhase = EBossPhase::Second;
		break;
	case EBossPhase::Second:
		CurrentPhase = EBossPhase::Third;
		break;
	case EBossPhase::Third:
		CurrentPhase = EBossPhase::End;
		break;
	case EBossPhase::End:
		break;
	default:
		CurrentPhase = EBossPhase::First;
		break;
	}

	FString EnumString = StaticEnum<EBossPhase>()->GetNameByValue(static_cast<int64>(CurrentPhase)).ToString();
	MC_LOG(LogMC, Log, TEXT("EnumValue : %s"), *EnumString);

	if (ANamelessTalGameMode* GameMode = Cast<ANamelessTalGameMode>(GetWorld()->GetAuthGameMode()))
	{
		GameMode->BossPhaseStarted(CurrentPhase);
	}
}

float ANTNPCCharacterBoss::ApplyDamage(AActor* DamageCauser, const FNPCApplyDamageData InDamageData)
{
	MC_LOG(LogMC, Log, TEXT("Begin"));

	float ResultDamage = Super::ApplyDamage(DamageCauser, InDamageData);

	return ResultDamage;
}


void ANTNPCCharacterBoss::SetAttackPatternDatas()
{
	const TArray<FNTNPCAttackPatternData> FindedPatternDatas = UNTGameDataSingleton::Get().FindNPCAttackPatternsByUser(PatternUserID);
	if (FindedPatternDatas.IsEmpty())
	{
		MC_LOG(LogMC, Warning, TEXT("No PatternData! %s"), *PatternUserID.ToString());

		PhasePatternDatas[EBossPhase::First].Patterns.Add({ TEXT("P_NormalMon1_Attack1_0"),  2.0f, 200.0f, 50, 0.0f });
		PhasePatternDatas[EBossPhase::Second].Patterns.Add({ TEXT("P_NormalMon1_Attack2_0"), 4.0f, 450.0f, 30, 0.0f });
		PhasePatternDatas[EBossPhase::Third].Patterns.Add({ TEXT("P_NormalMon1_Attack3_0"),   6.0f, 700.0f, 15, 0.0f });
	}
	else
	{
		for (FNTNPCAttackPatternData PatternData : FindedPatternDatas)
		{
			if (PatternData.PatternPhase == 1)
			{
				PhasePatternDatas[EBossPhase::First].Patterns.Add(FNPCAttackPatternSelectData(PatternData.FirstStepID, PatternData.CoolDown, PatternData.PatternRange, PatternData.PatternWeight, 0.0f));
			}
			else if (PatternData.PatternPhase == 2)
			{
				PhasePatternDatas[EBossPhase::Second].Patterns.Add(FNPCAttackPatternSelectData(PatternData.FirstStepID, PatternData.CoolDown, PatternData.PatternRange, PatternData.PatternWeight, 0.0f));
			}
			else
			{
				PhasePatternDatas[EBossPhase::Third].Patterns.Add(FNPCAttackPatternSelectData(PatternData.FirstStepID, PatternData.CoolDown, PatternData.PatternRange, PatternData.PatternWeight, 0.0f));
			}
		}
	}
}

TArray<FNPCAttackPatternSelectData>& ANTNPCCharacterBoss::GetAttackPatternDatas()
{
	return PhasePatternDatas[CurrentPhase].Patterns;
}

void ANTNPCCharacterBoss::HandleDeath()
{
	MC_LOG(LogMC, Log, TEXT("Begin"));

	if (bIsDead)
	{
		MC_LOG(LogMC, Error, TEXT("bIsDead is Already"));
		return;
	}

	bIsDead = true;

	if (UNTLockOnTargetComponent* LockOnTarget = FindComponentByClass<UNTLockOnTargetComponent>())
	{
		LockOnTarget->SetLockOnEnabled(false);
	}

	SetCanBeDamaged(false);

	bIsAttacking = false;
	SetActorEnableCollision(false);
	OnAICharacterAttackFinished.ExecuteIfBound();
	OnAICharacterAttackFinished.Unbind();

	StopCurrentPlayMontage();

	if (DeathMontage)
	{
		if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
		{
			AnimInstance->Montage_Play(DeathMontage);

			FOnMontageEnded EndDelegate;
			EndDelegate.BindUObject(this, &ANTNPCCharacterBoss::StartBossSequence);
			AnimInstance->Montage_SetEndDelegate(EndDelegate, DeathMontage);
		}
	}
	else
	{
		MC_LOG(LogSY, Warning, TEXT("DeathMontage is not assigned : %s"), *GetName());
	}

	MC_LOG(LogSY, Warning, TEXT("NPC DEAD : %s"), *GetName());
}

void ANTNPCCharacterBoss::StartNextPhase()
{
	MC_LOG(LogMC, Log, TEXT("Begin"));

	if (CurrentPhase == EBossPhase::End)
	{
		return;
	}

	StatComponent->MaxHP = PhaseHP[CurrentPhase];
	StatComponent->RestoreFull();
	StatComponent->SetSoulRegenBlocked(true);
	bIsDead = false;

	HeadringRotationSpeed = PhaseHeadringRotationSpeed[CurrentPhase];

	SetCanBeDamaged(true);
	SetActorEnableCollision(true);

	GetWorld()->GetTimerManager().SetTimer(PhaseStartDelayTimer, this, &ANTNPCCharacterBoss::ReleaseStun, 2.0f, false);

	// Phase 진입에 따른 흙/찢어짐/핏자국 파라미터 전환 (First=스폰 상태, Second=Phase1, Third=Phase2)
	if (CurrentPhase == EBossPhase::Second || CurrentPhase == EBossPhase::Third)
	{
		SetPartsScalarParameter(TEXT("DirtAmount"), 1.0f);
		SetPartsScalarParameter(TEXT("CutoutAmount"), 1.0f);
	}

	if (CurrentPhase == EBossPhase::Third)
	{
		SetPartsScalarParameter(TEXT("BloodAmount"), 1.0f);
	}

	FString EnumString = StaticEnum<EBossPhase>()->GetNameByValue(static_cast<int64>(CurrentPhase)).ToString();
	MC_LOG(LogMC, Log, TEXT("End : %s"), *EnumString);
}

void ANTNPCCharacterBoss::BeginAttack()
{
	Super::BeginAttack();

	if (BBComponent->GetValueAsName(BBKEY_SELECTEDATTACK) == TEXT("P_BossMon1_Attack4") || BBComponent->GetValueAsName(BBKEY_SELECTEDATTACK) == TEXT("P_BossMon1_Attack6"))
	{
		if (!OkJangPanClass)
		{
			MC_LOG(LogMC, Error, TEXT("OkJangPanClass is not set!"));
			return;
		}

		FVector SpawnLocation = GetActorLocation();
		SpawnLocation.Z -= GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		ANTOkJangPan* SpawnedOkJangPan = GetWorld()->SpawnActor<ANTOkJangPan>(OkJangPanClass, SpawnLocation, GetActorRotation(), SpawnParams);
		if (SpawnedOkJangPan)
		{
			SpawnedOkJangPan->StartAOE();
		}
	}
}
