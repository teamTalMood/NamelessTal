// Fill out your copyright notice in the Description page of Project Settings.


#include "NPC/NTNPCCharacterBoss.h"
#include "NPC/Animations/NTNPCAnimInstance.h"
#include "MCLOG.h"
#include "Game/NTGameDataSingleton.h"
#include "Stat/StatComponent.h"
#include "NamelessTalGameMode.h"
#include "NPC/Components/NTHPWidgetComponent.h"
#include "TalBase.h"
#include "Components/NTTalComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

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
	PhasePatternDatas.Add(EBossPhase::End, FNPCAttackPatternList());

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
	SpawnWaitingLeftTal();

	//CurrentPhase = EBossPhase::Third;

	if (ANamelessTalGameMode* GameMode = Cast<ANamelessTalGameMode>(GetWorld()->GetAuthGameMode()))
	{
		GameMode->OnCutsceneSequenceEnded.AddDynamic(this, &ANTNPCCharacterBoss::StartNextPhase);
	}

	StatComponent->OnHPChanged.AddUniqueDynamic(this, &ANTNPCCharacterBoss::NotifyGameModeHPChanged);
	StatComponent->OnSoulChanged.AddUniqueDynamic(this, &ANTNPCCharacterBoss::NotifyGameModeSoulChanged);

	StatComponent->MaxHP = PhaseHP[CurrentPhase];
	StatComponent->RestoreFull();
	StatComponent->SetSoulRegenBlocked(true);

	HeadringRotationSpeed = PhaseHeadringRotationSpeed[CurrentPhase];

	// 보스 공용 파츠(Top/Pants/Shoulder)는 기본값이 이미 더러운 상태(Dirt=1,Cutout=1)라
	// 스폰 시 깨끗한 상태로 시작하도록 명시적으로 0으로 덮어써야 함
	SetPartsScalarParameter(TEXT("DirtAmount"), 0.0f);
	SetPartsScalarParameter(TEXT("CutoutAmount"), 0.0f);
}

void ANTNPCCharacterBoss::SpawnWaitingLeftTal()
{
	if (IsValid(WaitingLeftTal) || !LeftTalClass || !GetWorld() || !IsValid(GetMesh()))
	{
		return;
	}

	if (!GetMesh()->DoesSocketExist(LeftTalSocketName))
	{
		UE_LOG(LogTemp, Warning, TEXT("Boss left Tal socket missing: %s"),
			*LeftTalSocketName.ToString());
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ATalBase* NewTal = GetWorld()->SpawnActor<ATalBase>(
		LeftTalClass, GetActorLocation(), GetActorRotation(), SpawnParams);
	if (!IsValid(NewTal)) return;

	const bool bAttached = NewTal->AttachToComponent(
		GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		LeftTalSocketName);
	if (!bAttached)
	{
		NewTal->Destroy();
		return;
	}

	NewTal->SetActorRelativeScale3D(LeftTalWearScale);
	WaitingLeftTal = NewTal;
}

void ANTNPCCharacterBoss::ActivateLeftTal()
{
	if (!IsValid(TalComponent) || !IsValid(WaitingLeftTal))
	{
		return;
	}

	if (!IsValid(GetMesh()) || !GetMesh()->DoesSocketExist(LeftTalSocketName))
	{
		UE_LOG(LogTemp, Warning, TEXT("Boss left Tal socket missing: %s"),
			*LeftTalSocketName.ToString());
		return;
	}

	// 탈취 없이 HP만 깎여 페이즈가 넘어온 경우, 남아 있는 이전 탈을 정리합니다.
	// Destroy() 전에 UnequipTal()을 호출해야 OnTalChanged(nullptr)이 정상 전파됩니다.
	if (TalComponent->GetCurrentTal())
	{
		if (ATalBase* RemovedTal = TalComponent->UnequipTal())
		{
			RemovedTal->Destroy();
		}
	}

	// 왼쪽 반쪽은 원점이 달라 전용 소켓을 사용합니다.
	// EquipTal()은 항상 TalSocketName으로 스냅하므로 등록 직전에 바꿔야 합니다.
	TalComponent->SetTalSocketName(LeftTalSocketName);

	if (TalComponent->EquipTal(WaitingLeftTal))
	{
		// 이후로는 TalComponent가 관리하므로 대기 참조를 비웁니다.
		WaitingLeftTal = nullptr;
	}
	else
	{
		// 실패 시 참조를 유지해야 EndPlay()에서 정리됩니다.
		UE_LOG(LogTemp, Warning, TEXT("Boss left Tal equip failed."));
	}
}

void ANTNPCCharacterBoss::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 플레이어에게 넘어간 탈은 제거하지 않습니다.
	if (IsValid(WaitingLeftTal) && WaitingLeftTal->GetOwner() == this)
	{
		WaitingLeftTal->Destroy();
	}
	WaitingLeftTal = nullptr;

	Super::EndPlay(EndPlayReason);
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
	//MC_LOG(LogMC, Log, TEXT("Begin"));

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
	//MC_LOG(LogMC, Log, TEXT("EnumValue : %s"), *EnumString);

	if (ANamelessTalGameMode* GameMode = Cast<ANamelessTalGameMode>(GetWorld()->GetAuthGameMode()))
	{
		GameMode->BossPhaseStarted(CurrentPhase);
	}
}

void ANTNPCCharacterBoss::SetupCharacterWidget(UNTHPBarWidget* InUserWidget)
{
	if (HpBar)
	{
		HpBar->SetVisibility(false);
	}
}

float ANTNPCCharacterBoss::ApplyDamage(AActor* DamageCauser, const FNPCApplyDamageData InDamageData)
{
	//MC_LOG(LogMC, Log, TEXT("Begin"));

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
	//MC_LOG(LogMC, Log, TEXT("Begin"));

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

	if (CurrentPhase != EBossPhase::Third)
	{
		StartBossSequence(nullptr, false);
		return;
	}

	// 여기부터는 진짜 사망. BT를 멈추지 않으면 CurrentPhase가 End로 바뀐 뒤에도
	// NTBTS_SelectPattern이 계속 돌면서 PhasePatternDatas[End]를 찾아 크래시난다
	if (BTComponent)
	{
		BTComponent->StopLogic(TEXT("Boss Dead"));
	}

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
}

void ANTNPCCharacterBoss::NotifyGameModeHPChanged(float NewHP, float MaxHP)
{
	if (ANamelessTalGameMode* GameMode = Cast<ANamelessTalGameMode>(GetWorld()->GetAuthGameMode()))
	{
		GameMode->NotifyBossHPChanged(NewHP, MaxHP);
	}
}

void ANTNPCCharacterBoss::NotifyGameModeSoulChanged(float NewSoul, float MaxSoul)
{
	if (ANamelessTalGameMode* GameMode = Cast<ANamelessTalGameMode>(GetWorld()->GetAuthGameMode()))
	{
		GameMode->NotifyBossSoulChanged(NewSoul, MaxSoul);
	}
}

void ANTNPCCharacterBoss::StartNextPhase()
{
	//MC_LOG(LogMC, Log, TEXT("Begin"));

	if (CurrentPhase == EBossPhase::End)
	{
		return;
	}

	// 이전 페이즈에서 남은 그로기 타이머와 탈취 회복 잠금을 정리합니다.
	GetWorldTimerManager().ClearTimer(ExhaustedTimerHandle);
	bTalchiHeld = false;
	bTalchiPausedExhaustedTimer = false;
	bTalchiDeferredExhaustedEnd = false;
	StatComponent->SetTalchiRecoveryPaused(false);

	bIsExhausting = false;
	if (UNTNPCAnimInstance* Anim = Cast<UNTNPCAnimInstance>(GetMesh()->GetAnimInstance()))
	{
		Anim->SetAnimIsExhausted(false);
	}

	StatComponent->MaxHP = PhaseHP[CurrentPhase];
	StatComponent->RestoreFull();
	StatComponent->SetSoulRegenBlocked(true);
	bIsDead = false;

	HeadringRotationSpeed = PhaseHeadringRotationSpeed[CurrentPhase];

	SetCanBeDamaged(true);
	SetActorEnableCollision(true);

	// HandleDeath()가 페이즈 종료 때 껐던 락온을 되살립니다.
	// CurrentPhase == End면 위에서 return하므로 진짜 사망 시에는 실행되지 않습니다.
	if (UNTLockOnTargetComponent* LockOnTarget = FindComponentByClass<UNTLockOnTargetComponent>())
	{
		LockOnTarget->SetLockOnEnabled(true);
	}

	// 2페이즈부터 왼쪽 탈을 탈취 대상으로 등록합니다.
	if (CurrentPhase == EBossPhase::Second)
	{
		ActivateLeftTal();
	}

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
	//MC_LOG(LogMC, Log, TEXT("End : %s"), *EnumString);
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
