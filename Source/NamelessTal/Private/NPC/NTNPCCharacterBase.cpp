// Fill out your copyright notice in the Description page of Project Settings.


#include "NPC/NTNPCCharacterBase.h"
#include "MCLOG.h"

//Components
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "NPC/Components/NTLockOnTargetComponent.h"
#include "NPC/Components/NTTalChiTargetComponent.h"

//Engine
#include "Kismet/GameplayStatics.h"
#include "Animation/AnimMontage.h"

//AI
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

//NT
#include "NPC/NTAIController.h"
#include "Stat/StatComponent.h"
#include "NPC/Animations/NTNPCAnimInstance.h"
#include "NPC/NTAIParams.h"
#include "NPC/Components/NTLockOnTargetComponent.h"
#include "Game/NTGameDataSingleton.h"
#include "NPC/NTNPCAttackPatternStepData.h"

//Interface
#include "Player/Interface/NTPlayerApplyDamageInterface.h"

//Tal
#include "TalBase.h"
#include "Components/NTTalComponent.h"

//State
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
ANTNPCCharacterBase::ANTNPCCharacterBase()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	GetMesh()->SetRelativeLocationAndRotation(FVector(0, 0, -GetCapsuleComponent()->GetScaledCapsuleHalfHeight()), FRotator(0, -90, 0));

	// AIController
	AIControllerClass = ANTAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	// 무기 Mesh Set
	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetupAttachment(GetMesh(), TEXT("WeaponSocket_R"));
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	WeaponCollision = CreateDefaultSubobject<UCapsuleComponent>(TEXT("WeaponCollision"));
	WeaponCollision->ComponentTags.Add(TEXT("WeaponCollision"));
	WeaponCollision->SetupAttachment(WeaponMesh);
	WeaponCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	WeaponCollision->OnComponentBeginOverlap.AddDynamic(this, &ANTNPCCharacterBase::OnWeaponOverlap);

	// Stat
	StatComponent = CreateDefaultSubobject<UStatComponent>(TEXT("StatComponent"));
	StatComponent->SetSoulRegenBlocked(true);

	// Lock On
	LockOnTargetComponent = CreateDefaultSubobject<UNTLockOnTargetComponent>(TEXT("LockOnTargetCom"));

	// TalChi Target
	TalChiTargetComponent = CreateDefaultSubobject<UNTTalChiTargetComponent>(TEXT("TalChiTargetCom"));
	TalChiTargetComponent->SetupAttachment(GetCapsuleComponent());

	// Attack Direction Set
	AttackDirections.Add(EAttackDistance::AttackNone, 0);
	AttackDirections.Add(EAttackDistance::AttackShort, 0);
	AttackDirections.Add(EAttackDistance::AttackMiddle, 0);
	AttackDirections.Add(EAttackDistance::AttackLong, 0);

	TalComponent = CreateDefaultSubobject<UNTTalComponent>(TEXT("TalComponent"));
}

// Called when the game starts or when spawned
void ANTNPCCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	AAIController* AICon = GetController<AAIController>();
	BTComponent = Cast<UBehaviorTreeComponent>(AICon->GetBrainComponent());
	BBComponent = AICon->GetBlackboardComponent();

	// 현재위치를 HomePos로 지정
	if (BBComponent)
	{
		BBComponent->SetValueAsVector(BBKEY_HOMEPOS, GetActorLocation());
		BBComponent->SetValueAsName(BBKEY_SELECTEDATTACK, BB_ATTACKPATTERN_NONE);
		BBComponent->SetValueAsVector(BBKEY_ATTACKPOS, GetActorLocation());
	}

	if (TalComponent && DefaultTalClass)
	{
		TalComponent->SpawnDefaultTal(DefaultTalClass);
	}

	if (StatComponent)
	{
		StatComponent->OnDeath.AddUniqueDynamic(this, &ANTNPCCharacterBase::HandleDeath);
	}

	const TArray<FNTNPCAttackPatternData> FindedPatternDatas = UNTGameDataSingleton::Get().FindNPCAttackPatternsByUser(PatternUserID);
	if (FindedPatternDatas.IsEmpty())
	{
		MC_LOG(LogMC, Warning, TEXT("No PatternData! %s"), *PatternUserID.ToString());

		AttackPatternDatas = {
			{ TEXT("P_NormalMon1_Attack1_0"),  2.0f, 200.0f, 50, 0.0f },
			{ TEXT("P_NormalMon1_Attack2_0"), 4.0f, 450.0f, 30, 0.0f },
			{ TEXT("P_NormalMon1_Attack3_0"),   6.0f, 700.0f, 15, 0.0f },
		};
	}
	else
	{
		for (FNTNPCAttackPatternData PatternData : FindedPatternDatas)
		{
			AttackPatternDatas.Add(FNPCAttackPatternSelectData(PatternData.FirstStepID, PatternData.CoolDown, PatternData.PatternRange, PatternData.PatternWeight, 0.0f));
		}
	}

}

void ANTNPCCharacterBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 0. 델리게이트 빼기
	if (StatComponent)
	{
		StatComponent->OnDeath.RemoveDynamic(this, &ANTNPCCharacterBase::HandleDeath);
	}
	// 1. 무기 콜리전 오버랩 델리게이트 해제
	if (WeaponCollision)
	{
		WeaponCollision->OnComponentBeginOverlap.RemoveDynamic(this, &ANTNPCCharacterBase::OnWeaponOverlap);
	}

	// 2. 애님 몽타주 End 델리게이트 해제 (공격/피격 둘 다)
	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		if (UAnimInstance* AnimInstance = MeshComp->GetAnimInstance())
		{
			static FOnMontageEnded EmptyDelegate;

			AnimInstance->Montage_SetEndDelegate(EmptyDelegate, DamageMontage);
			AnimInstance->Montage_SetEndDelegate(EmptyDelegate, JumpMontage);

			// AttackMontages의 델리게이트 해제
			for (const auto& AttackMontagePair : AttackMontages)
			{
				AnimInstance->Montage_SetEndDelegate(EmptyDelegate, AttackMontagePair.Value);
			}

			// 재생 중인 몽타주도 즉시 정지
			AnimInstance->StopAllMontages(0.0f);
		}
	}

	// 3. AI 공격 완료 델리게이트 초기화 (AIController 등 외부에서 바인딩했을 수 있음)
	OnAICharacterAttackFinished.Unbind();

	// 4. BTComponent 정지 (있다면)
	if (BTComponent)
	{
		BTComponent->StopLogic(TEXT("EndPlay"));
	}

	Super::EndPlay(EndPlayReason);
}

void ANTNPCCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bISAttackMoving)
	{
		AddActorWorldOffset(AttackMoving * DeltaTime, true);
	}
}

// Called to bind functionality to input
void ANTNPCCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

float ANTNPCCharacterBase::GetAIPatrolRadius()
{
	return AIPatrolRadius;
}

float ANTNPCCharacterBase::GetAIDetectRange()
{
	return AIDetectRange;
}

float ANTNPCCharacterBase::GetAIAttackRange()
{
	return AIAttackRange;
}

float ANTNPCCharacterBase::GetAITurnSpeed()
{
	return AITurnSpeed;
}

void ANTNPCCharacterBase::AttackByAI()
{
	MC_LOG(LogMC, Log, TEXT("Begin"));

	BTComponent->PauseLogic(TEXT("BeginAttack"));

	if (bIsDead)
	{
		return;
	}

	// 타겟 방향으로 바라보기
	ACharacter* LocalPlayerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	FVector Direction = LocalPlayerCharacter->GetActorLocation() - GetActorLocation();
	FRotator NewRotation = FRotationMatrix::MakeFromX(Direction).Rotator();
	NewRotation.Pitch = 0.f; NewRotation.Roll = 0.f; // 눕지 않게 고정
	SetActorRotation(NewRotation);

	// 공격 위치로 점프 -> 이 함수 내부에서 공격 시작 콜
	MoveToAttackPoint();
}

void ANTNPCCharacterBase::SetAIAttackDelegate(const FAICharacterAttackFinished& InOnAttackFinished)
{
	OnAICharacterAttackFinished = InOnAttackFinished;
}

float ANTNPCCharacterBase::ApplyDamage(AActor* DamageCauser, const float InDamage, const float InDrain, const bool bIsParry)
{
	FNPCApplyDamageData ApplyDamageData = { InDamage, 100, 5.0f, bIsParry };

	return ApplyDamage(DamageCauser, ApplyDamageData);
}

float ANTNPCCharacterBase::ApplyDamage(AActor* DamageCauser, const FNPCApplyDamageData InDamageData)
{
	//MC_LOG(LogMC, Log, TEXT("Begin"));

	////////////////////////////////
	// 데미지 적용 전 처리

	if (BTComponent == nullptr || bIsDead || !StatComponent)
	{
		return 0.0f;
	}


	////////////////////////////////
	// 패링일 때 처리
	if (InDamageData.bIsParry)
	{
		// 패링일때는 HP뎀 없고 넋뎀은 현재 공격중인 넋뎀의 50퍼
		StatComponent->DrainSoul(CurrentAttackData.DamageSoul * 0.7f);

		if (bIsAttacking)
		{
			// 공격 로직 초기화
			StartStun();
		}

		StartDamaged();
	}


	////////////////////////////////
	// 피격일 때 처리
	if (!InDamageData.bIsParry)
	{
		// 피격시(플레이어->NPC공격)일때는 플레이어가 주는 수치대로 넋뎀 HP뎀
		StatComponent->DrainSoul(InDamageData.SoulDamage);
		StatComponent->ApplyDamage(InDamageData.HPDamage);

		// 만약 피해받고 죽었으면 피격 처리 중단
		if (bIsDead)
		{
			//MC_LOG(LogMC, Log, TEXT("Get Deaded"));
			return -InDamageData.HPDamage;
		}

		// 공격중단 판단 : 스턴 이거나 그로기 진입
		if (bIsAttacking)
		{
			CurrentStunRate += InDamageData.StaggerDamage;
			if (CurrentStunRate >= CurrentAttackData.HitStun || StatComponent->IsSoulExhausted())
			{
				StartStun();
				StartDamaged();
			}
		}

		// 그냥 피격체크 - 그로기가 아닐때만 데미지 애니메이션 적용
		if (!bIsExhausting)
		{
			StartStun();
			StartDamaged();
		}
	}

	//MC_LOG(LogMC, Log, TEXT("End"));

	return InDamageData.HPDamage;
}

uint8 ANTNPCCharacterBase::GetAttackDirection()
{
	return 2;
}

TArray<FNPCAttackPatternSelectData>& ANTNPCCharacterBase::GetAttackPatternDatas()
{
	return AttackPatternDatas;
}

const uint8 ANTNPCCharacterBase::GetIsExhausting()
{
	return bIsExhausting;
}

void ANTNPCCharacterBase::TalChiNPC(FNPCTalchiData InTalchiData)
{
	WeaponMesh->SetVisibility(false);
	
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance == nullptr)
	{
		return;
	}

	static FOnMontageEnded EmptyDelegate;
	UAnimMontage* CurrentMontage = AnimInstance->GetCurrentActiveMontage();
	if (CurrentMontage)
	{
		AnimInstance->Montage_SetEndDelegate(EmptyDelegate, CurrentMontage);
	}
	AnimInstance->Montage_Stop(0.1f);

	FOnMontageEnded EndDelegate;
	EndDelegate.BindUObject(this, &ANTNPCCharacterBase::OnTalChiMontageEnd);
	AnimInstance->Montage_SetEndDelegate(EndDelegate, TalChiMontage);
}

void ANTNPCCharacterBase::EnableWeaponCollision()
{
	if (WeaponCollision == nullptr)
	{
		MC_LOG(LogMC, Error, TEXT("No WeaponCollision!"));
		return;
	}
	HitActors.Empty(); // 새 공격이라 이전 기록 비우기
	WeaponCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	WeaponCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
}

void ANTNPCCharacterBase::DisableWeaponCollision()
{
	if (WeaponCollision == nullptr)
	{
		MC_LOG(LogMC, Error, TEXT("No WeaponCollision!"));
		return;
	}
	WeaponCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ANTNPCCharacterBase::MoveToAttackPoint()
{
	//MC_LOG(LogMC, Log, TEXT("Begin"));

	if (JumpMontage == nullptr)
	{
		MC_LOG(LogMC, Error, TEXT("No JumpMontage!!"));
		return;
	}

	if (JumpMontageSpeedRate == 0)
	{
		MC_LOG(LogMC, Error, TEXT("JumpMontageSpeedRate is 0!!"));
		return;
	}

	//점프 거리 계산
	const float JumpMontageDuration = JumpMontage->GetPlayLength() / JumpMontageSpeedRate;
	const FVector TargetAttackPos = BTComponent->GetBlackboardComponent()->GetValueAsVector(BBKEY_ATTACKPOS);

#if ENABLE_DRAW_DEBUG
	DrawDebugSphere(GetWorld(), TargetAttackPos, 20.f, 16, FColor::Red, false, JumpMontageDuration);
#endif

	const FVector ToTarget = TargetAttackPos - GetActorLocation();
	const float DistanceToTarget = ToTarget.Length();
	const FVector DirectionToTarget = ToTarget.GetSafeNormal();

	const float ForwardDot = FVector::DotProduct(GetActorForwardVector(), DirectionToTarget);

	if (DistanceToTarget <= AttackDistance || ForwardDot <= 0.f)
	{
		OnJumpMontageEnd(nullptr, false);
		return;
	}

	const float MoveDistance = DistanceToTarget - AttackDistance;
	AttackMoving = (JumpMontageDuration > 0.f) ? (DirectionToTarget * MoveDistance) / JumpMontageDuration : FVector::ZeroVector;

	//MC_LOG(LogMC, Log, TEXT("JumpMontageDuration : %f"), JumpMontageDuration);
	//MC_LOG(LogMC, Log, TEXT("TargetAttackPos : %f, %f, %f"), TargetAttackPos.X, TargetAttackPos.Y, TargetAttackPos.Z);
	//MC_LOG(LogMC, Log, TEXT("AttackMoving : %f, %f, %f"), AttackMoving.X, AttackMoving.Y, AttackMoving.Z);

	bISAttackMoving = true;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	AnimInstance->Montage_Play(JumpMontage, JumpMontageSpeedRate);

	FOnMontageEnded EndDelegate;
	EndDelegate.BindUObject(this, &ANTNPCCharacterBase::OnJumpMontageEnd);
	AnimInstance->Montage_SetEndDelegate(EndDelegate, JumpMontage);
}

void ANTNPCCharacterBase::OnJumpMontageEnd(UAnimMontage* TargetMontage, bool IsProperlyEnded)
{
	bISAttackMoving = false;

	//MC_LOG(LogMC, Log, TEXT("Begin"));
	BeginAttack();
}

void ANTNPCCharacterBase::BeginAttack()
{
	MC_LOG(LogMC, Log, TEXT("Begin"));

	if (bIsDead)
	{
		return;
	}

	// 현재 공격 스텝 데이터 갖고오기
	FName CurrentAttackStep = BBComponent->GetValueAsName(BBKEY_SELECTEDATTACK);
	const FNTNPCAttackPatternStepData* PatternStepData = UNTGameDataSingleton::Get().GetNPCAttackPatternStep(CurrentAttackStep);

	if (PatternStepData == nullptr)
	{
		MC_LOG(LogMC, Error, TEXT("No PatternStepData! %s"), *CurrentAttackStep.ToString());

		CurrentAttackData.NextStepID = FName(TEXT(""));
		CurrentAttackData.DamageHP = 10.0f;
		CurrentAttackData.DamageSoul = 10.0f;
		CurrentAttackData.GuardBack = 1.0f;
		CurrentAttackData.bCriticalEnable = true;
		CurrentAttackData.HitStun = 3.0f;
		CurrentAttackData.RecoveryTime = 1.0f;
		CurrentAttackData.AnimationClip = FName(TEXT("AM_NTMon1Attack1"));
	}
	else
	{
		CurrentAttackData.NextStepID = PatternStepData->NextStepID;
		CurrentAttackData.DamageHP = PatternStepData->DamageHP;
		CurrentAttackData.DamageSoul = PatternStepData->DamageSoul;
		CurrentAttackData.GuardBack = PatternStepData->GuardBack;
		CurrentAttackData.bCriticalEnable = PatternStepData->bCriticalEnable;
		CurrentAttackData.HitStun = PatternStepData->HitStun;
		CurrentAttackData.RecoveryTime = PatternStepData->RecoveryTime;
		CurrentAttackData.AnimationClip = PatternStepData->AnimationClip;
	}

	// Animation Setting
	const float AttackSpeedRate = 1.0f;
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	TObjectPtr<UAnimMontage>* FoundMontage = AttackMontages.Find(CurrentAttackData.AnimationClip);
	if (!FoundMontage || !*FoundMontage)
	{
		MC_LOG(LogMC, Error, TEXT("No AttackMontage!!"));
		return;
	}
	UAnimMontage* AttackMontage = *FoundMontage;
	bIsAttacking = true;
	CurrentStunRate = 0.0f;

	AnimInstance->Montage_Play(AttackMontage, AttackSpeedRate);

	FOnMontageEnded EndDelegate;
	EndDelegate.BindUObject(this, &ANTNPCCharacterBase::OnAttackMontageEnd);
	AnimInstance->Montage_SetEndDelegate(EndDelegate, AttackMontage);
}

void ANTNPCCharacterBase::OnWeaponOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 떄려지는 객체 확인 로그
	if (OtherActor == nullptr || OtherActor == this || HitActors.Contains(OtherActor))
	{
		return;
	}
	//MC_LOG(LogMC, Log, TEXT("Weapon overlapped with: %s"), *GetNameSafe(OtherActor));

	if (OtherActor != UGameplayStatics::GetPlayerCharacter(GetWorld(), 0))
	{
		return;
	}

	INTPlayerApplyDamageInterface* PlayerCharacter = Cast<INTPlayerApplyDamageInterface>(OtherActor);
	if (PlayerCharacter == nullptr)
	{
		return;
	}

	if (OtherComp && OtherComp->ComponentHasTag(TEXT("WeaponCollision")))
	{
		//MC_LOG(LogMC, Log, TEXT("WeaponCollision"));
		return; // 무기끼리 부딪힌 경우는 데미지 적용 안 함
	}

	HitActors.Add(OtherActor);

	FPlayerApplyDamageData DamageData;
	DamageData.HPDamage = CurrentAttackData.DamageHP;
	DamageData.SoulDamage = CurrentAttackData.DamageSoul;
	DamageData.GuardBack = CurrentAttackData.GuardBack;
	DamageData.bCriticalEnable = CurrentAttackData.bCriticalEnable;
	DamageData.PlayerRecoveryTime = CurrentAttackData.RecoveryTime;

	PlayerCharacter->ApplyDamage(this, DamageData);

	//const FVector ImpactPoint = SweepResult.bBlockingHit ? FVector(SweepResult.ImpactPoint) : OtherActor->GetActorLocation();
	//Damageable->ApplyDamage(AttackDamage, this, ImpactPoint, GetActorForwardVector() * 200.0f);
	//UGameplayStatics::ApplyDamage(OtherActor, AttackDamage, GetController(), this, UDamageType::StaticClass());
}

void ANTNPCCharacterBase::OnAttackMontageEnd(UAnimMontage* TargetMontage, bool IsProperlyEnded)
{
	//MC_LOG(LogMC, Log, TEXT("Begin"));

	if (CurrentAttackData.NextStepID == BB_ATTACKSTEP_NONE)
	{
		// 연계공격이 없으면
		EndAttack();
		return;
	}

	BBComponent->SetValueAsName(BBKEY_SELECTEDATTACK, CurrentAttackData.NextStepID);
	BeginAttack();
}

void ANTNPCCharacterBase::EndAttack()
{
	//MC_LOG(LogMC, Log, TEXT("Begin"));

	BBComponent->SetValueAsName(BBKEY_SELECTEDATTACK, BB_ATTACKPATTERN_NONE);

	bIsAttacking = false;
	//CurrentAttack = EAttackDistance::AttackNone;
	BTComponent->ResumeLogic(TEXT("Attack End"));
	OnAICharacterAttackFinished.ExecuteIfBound();
	return;
}

void ANTNPCCharacterBase::StartStun()
{
	//MC_LOG(LogMC, Log, TEXT("Begin"));

	BBComponent->SetValueAsName(BBKEY_SELECTEDATTACK, BB_ATTACKPATTERN_NONE);
	bIsAttacking = false;
	OnAICharacterAttackFinished.ExecuteIfBound();
	BTComponent->PauseLogic(TEXT("Start Stun"));

	// 현재 실행중인 몽타주강 있다면 EndDelegate 해제하고 멈추기
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance == nullptr)
	{
		return;
	}

	static FOnMontageEnded EmptyDelegate;
	UAnimMontage* CurrentMontage = AnimInstance->GetCurrentActiveMontage();
	if (CurrentMontage)
	{
		AnimInstance->Montage_SetEndDelegate(EmptyDelegate, CurrentMontage);
	}
	AnimInstance->Montage_Stop(0.1f);
}

void ANTNPCCharacterBase::ReleaseStun()
{
	//MC_LOG(LogMC, Log, TEXT("Begin"));

	BTComponent->ResumeLogic(TEXT("Stun End"));
}

void ANTNPCCharacterBase::EndExhausted()
{
	//MC_LOG(LogMC, Log, TEXT("Begin"));

	if (bIsDead)
	{
		return;
	}

	UNTNPCAnimInstance* Anim = Cast<UNTNPCAnimInstance>(GetMesh()->GetAnimInstance());
	Anim->SetAnimIsExhausted(false);
	StatComponent->RestoreSoul();
	bIsExhausting = false;

	FTimerHandle Handle;
	GetWorldTimerManager().SetTimer(Handle, this, &ANTNPCCharacterBase::ReleaseStun, 1.0f, false);
}

void ANTNPCCharacterBase::StartDamaged()
{
	if (DamageMontage == nullptr)
	{
		return;
	}

	// 피격 애니메이션 실행
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	AnimInstance->Montage_Play(DamageMontage, DamageSpeedRate);

	FOnMontageEnded EndDelegate;
	EndDelegate.BindUObject(this, &ANTNPCCharacterBase::OnDamageMontageEnd);
	AnimInstance->Montage_SetEndDelegate(EndDelegate, DamageMontage);
}

void ANTNPCCharacterBase::OnDamageMontageEnd(UAnimMontage* TargetMontage, bool IsProperlyEnded)
{
	////////////////////////////////
	// 데미지 사후 처리

	bool bIsSoulExhausted = StatComponent->IsSoulExhausted();

	// 그로기 체크
	if (bIsSoulExhausted && !bIsExhausting)
	{
		//MC_LOG(LogMC, Log, TEXT("Exhausted"));

		bIsExhausting = true;

		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		UNTNPCAnimInstance* NPCAnim = Cast<UNTNPCAnimInstance>(AnimInstance);
		if (NPCAnim == nullptr)
		{
			return;
		}

		NPCAnim->SetAnimIsExhausted(true);

		FTimerHandle Handle;
		GetWorldTimerManager().SetTimer(Handle, this, &ANTNPCCharacterBase::EndExhausted, ExhaustedDuration, false);
	}
	else
	{
		ReleaseStun();
	}
}

void ANTNPCCharacterBase::OnTalChiMontageEnd(UAnimMontage* TargetMontage, bool IsProperlyEnded)
{
	StatComponent->ApplyDamage(StatComponent->MaxHP * 0.5f);

	if (!bIsDead)
	{
		EndExhausted();
	}
}

void ANTNPCCharacterBase::HandleDeath()
{
	//MC_LOG(LogMC, Log, TEXT("Begin"));

	if (bIsDead)
	{
		MC_LOG(LogMC, Error, TEXT("bIsDead is Already"));
		return;
	}

	bIsDead = true;

	//락온 해제
	if (UNTLockOnTargetComponent* LockOnTarget = FindComponentByClass<UNTLockOnTargetComponent>())
	{
		LockOnTarget->SetLockOnEnabled(false);
	}

	SetCanBeDamaged(false);

	//공격 상태 종료
	bIsAttacking = false;
	//CurrentAttack = EAttackDistance::AttackNone;//상태변환
	SetActorEnableCollision(false);//갖고있는 콜리전 다 해제
	//DisableWeaponCollision();//콜리전해제
	OnAICharacterAttackFinished.Unbind();//바인드해제

	//재생중인 몽타주 종료 + 바인딩된 종료 델리게이트 해제 (사망 중 콤보 재호출 방지)
	if (UAnimInstance* CurrentAnimInstance = GetMesh()->GetAnimInstance())
	{
		if (UAnimMontage* CurrentMontage = CurrentAnimInstance->GetCurrentActiveMontage())
		{
			FOnMontageEnded EmptyEndDelegate;
			CurrentAnimInstance->Montage_SetEndDelegate(EmptyEndDelegate, CurrentMontage);
			CurrentAnimInstance->Montage_Stop(0.0f, CurrentMontage);
		}
	}

	if (BTComponent)
	{
		BTComponent->StopLogic(TEXT("NPC Dead"));
	}

	//이동정지 + 죽는 순간 바닥 노멀 계산
	FVector GroundNormal = FVector::UpVector;
	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		if (MovementComponent->CurrentFloor.bWalkableFloor)
		{
			GroundNormal = MovementComponent->CurrentFloor.HitResult.ImpactNormal;
		}
		else
		{
			FHitResult HitResult;
			FCollisionQueryParams QueryParams;
			QueryParams.AddIgnoredActor(this);
			const FVector TraceStart = GetActorLocation();
			const FVector TraceEnd = TraceStart - FVector(0.f, 0.f, GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + 200.f);
			if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, QueryParams))
			{
				GroundNormal = HitResult.ImpactNormal;
			}
		}

		MovementComponent->StopMovementImmediately();
		MovementComponent->DisableMovement();
	}

	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		if (UNTNPCAnimInstance* NPCAnim = Cast<UNTNPCAnimInstance>(AnimInstance))
		{
			const bool bInvertPitchSign = FVector::DotProduct(GetActorForwardVector(), GroundNormal) < 0.0f;
			NPCAnim->SetAnimDeaded(GroundNormal, GetActorForwardVector(), bInvertPitchSign, GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * 0.2f);
		}
	}

	if (DeathMontage)
	{
		if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
		{
			AnimInstance->Montage_Play(DeathMontage);
		}
	}
	else
	{
		MC_LOG(LogSY, Warning, TEXT("DeathMontage is not assigned : %s"), *GetName());
	}


	MC_LOG(LogSY, Warning, TEXT("NPC DEAD : %s"), *GetName());
}