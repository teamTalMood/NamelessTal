// Fill out your copyright notice in the Description page of Project Settings.


#include "NPC/NTNPCCharacterBase.h"
#include "MCLOG.h"

//Components
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"

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

//Interface
#include "Player/Interface/NTPlayerApplyDamageInterface.h"

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
	WeaponCollision->SetupAttachment(WeaponMesh);
	WeaponCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	WeaponCollision->OnComponentBeginOverlap.AddDynamic(this, &ANTNPCCharacterBase::OnWeaponOverlap);

	// Stat
	StatComponent = CreateDefaultSubobject<UStatComponent>(TEXT("StatComponent"));
	StatComponent->SetSoulRegenBlocked(true);

	// Attack Direction Set
	AttackDirections.Add(EAttackDistance::AttackNone, 0);
	AttackDirections.Add(EAttackDistance::AttackShort, 0);
	AttackDirections.Add(EAttackDistance::AttackMiddle, 0);
	AttackDirections.Add(EAttackDistance::AttackLong, 0);

	AttackParryables.Add(EAttackDistance::AttackNone, false);
	AttackParryables.Add(EAttackDistance::AttackShort, false);
	AttackParryables.Add(EAttackDistance::AttackMiddle, false);
	AttackParryables.Add(EAttackDistance::AttackLong, false);
}

// Called when the game starts or when spawned
void ANTNPCCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	AAIController* AICon = GetController<AAIController>();
	BTComponent = Cast<UBehaviorTreeComponent>(AICon->GetBrainComponent());

	// Attack Montage Set
	AttackMontages.Add(EAttackDistance::AttackShort, ShortAttackMontage);
	AttackMontages.Add(EAttackDistance::AttackMiddle, MiddleAttackMontage);
	AttackMontages.Add(EAttackDistance::AttackLong, LongAttackMontage);

	// 현재위치를 HomePos로 지정
	if (UBlackboardComponent* BB = AICon->GetBlackboardComponent())
	{
		BB->SetValueAsVector(TEXT("HomePos"), GetActorLocation());
	}
}

void ANTNPCCharacterBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
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
			AnimInstance->Montage_SetEndDelegate(EmptyDelegate, ShortAttackMontage);
			AnimInstance->Montage_SetEndDelegate(EmptyDelegate, MiddleAttackMontage);
			AnimInstance->Montage_SetEndDelegate(EmptyDelegate, LongAttackMontage);
			AnimInstance->Montage_SetEndDelegate(EmptyDelegate, DamageMontage);

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

/*
float ANTNPCCharacterBase::TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);

	MC_LOG(LogMC, Log, TEXT("Begin"));

	if (BTComponent == nullptr)
	{
		return 0.0f;
	}

	if (bIsAttacking)
	{
		// 공격 로직 초기화
		EndAttack();
	}

	// 스탯 적용
	StatComponent->ApplyDamage(Damage);

	//MC_LOG(LogMC, Log, TEXT("BTComponent PauseLogic"));
	BTComponent->PauseLogic(TEXT("Damaged"));

	if (DamageMontage == nullptr)
	{
		return 0.0f;
	}

	// 피격 애니메이션 실행
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	AnimInstance->Montage_Play(DamageMontage, DamageSpeedRate);

	FOnMontageEnded EndDelegate;
	EndDelegate.BindUObject(this, &ANTNPCCharacterBase::OnDamageMontageEnd);
	AnimInstance->Montage_SetEndDelegate(EndDelegate, DamageMontage);

	return Damage;
}
*/

// Called every frame
void ANTNPCCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

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
	//MC_LOG(LogMC, Log, TEXT("Begin"));

	// 거리 계산
	ACharacter* LocalPlayerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	float Dist = FVector::Dist(GetActorLocation(), LocalPlayerCharacter->GetActorLocation());

	EAttackDistance ResultAttckType = EAttackDistance::AttackNone;

	// 근거리
	if (Dist <= AttackRangeShort) // 2.0m
	{
		//MC_LOG(LogMC, Log, TEXT("근거리 공격 수행"));

		float Rand = FMath::FRandRange(0.f, 100.f);

		if (Rand < 50.f)       ResultAttckType = EAttackDistance::AttackShort;
		else if (Rand < 90.f)  ResultAttckType = EAttackDistance::AttackMiddle;
		else                   ResultAttckType = EAttackDistance::AttackLong;

#if ENABLE_DRAW_DEBUG
		FVector Center = GetActorLocation();
		UWorld* World = GetWorld();
		DrawDebugSphere(World, Center, AttackRangeShort, 16, FColor::Yellow, false, 2.0f);
#endif
	}

	// 중거리
	if (Dist > AttackRangeShort && Dist <= AttackRangeMiddle) // 2.0m ~ 4.5m
	{
		//MC_LOG(LogMC, Log, TEXT("중거리 공격 수행"));

		float Rand = FMath::FRandRange(0.f, 100.f);

		if (Rand < 20.f)       ResultAttckType = EAttackDistance::AttackShort;
		else if (Rand < 30.f)  ResultAttckType = EAttackDistance::AttackMiddle;
		else                   ResultAttckType = EAttackDistance::AttackLong;

#if ENABLE_DRAW_DEBUG
		FVector Center = GetActorLocation();
		UWorld* World = GetWorld();
		DrawDebugSphere(World, Center, AttackRangeMiddle, 16, FColor::Purple, false, 2.0f);
#endif
	}

	if (Dist > AttackRangeMiddle) // 4.5m ~ 7m
	{
		//MC_LOG(LogMC, Log, TEXT("원거리 공격 수행"));
		ResultAttckType = EAttackDistance::AttackLong;

#if ENABLE_DRAW_DEBUG
		FVector Center = GetActorLocation();
		UWorld* World = GetWorld();
		DrawDebugSphere(World, Center, AIAttackRange, 16, FColor::Blue, false, 2.0f);
#endif
	}

	BeginAttack(ResultAttckType);
}

void ANTNPCCharacterBase::SetAIAttackDelegate(const FAICharacterAttackFinished& InOnAttackFinished)
{
	OnAICharacterAttackFinished = InOnAttackFinished;
}

float ANTNPCCharacterBase::ApplyDamage(AActor* DamageCauser, const float InDamage, const float InDrain, const bool bIsParry)
{
	MC_LOG(LogMC, Log, TEXT("Begin"));

	if (BTComponent == nullptr)
	{
		return 0.0f;
	}

	if (bIsAttacking)
	{
		// 공격 로직 초기화
		EndAttack();
	}

	// 스탯 적용
	StatComponent->ApplyDamage(InDamage);
	StatComponent->DrainSoul(InDrain);

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance == nullptr)
	{
		return 0.0f;
	}

	BTComponent->PauseLogic(TEXT("Damaged"));
	AnimInstance->Montage_Stop(0.1f);

	// 그로기 처리
	if(StatComponent->IsSoulExhausted())
	{
		MC_LOG(LogMC, Log, TEXT("Exhausted"));
		UNTNPCAnimInstance* NPCAnim = Cast<UNTNPCAnimInstance>(AnimInstance);
		if (NPCAnim == nullptr)
		{
			return 0.0f;
		}

		NPCAnim->SetAnimIsExhausted(true);
		FTimerHandle Handle;
		GetWorld()->GetTimerManager().SetTimer(Handle, FTimerDelegate::CreateLambda([&]
			{
				MC_LOG(LogMC, Log, TEXT("Soul Restored"));
				UNTNPCAnimInstance* Anim = Cast<UNTNPCAnimInstance>(GetMesh()->GetAnimInstance());
				Anim->SetAnimIsExhausted(false);
				BTComponent->ResumeLogic(TEXT("Exhausted End"));
				StatComponent->RestoreSoul();
			}
		), ExhaustedDuration, false);
	}

	// Damaged처리
	if (!StatComponent->IsSoulExhausted())
	{
		//MC_LOG(LogMC, Log, TEXT("BTComponent PauseLogic"));

		if (DamageMontage == nullptr)
		{
			return 0.0f;
		}

		// 피격 애니메이션 실행

		AnimInstance->Montage_Play(DamageMontage, DamageSpeedRate);

		FOnMontageEnded EndDelegate;
		EndDelegate.BindUObject(this, &ANTNPCCharacterBase::OnDamageMontageEnd);
		AnimInstance->Montage_SetEndDelegate(EndDelegate, DamageMontage);
	}

	return InDamage;
}

uint8 ANTNPCCharacterBase::GetAttackDirection()
{
	return AttackDirections[CurrentAttack];
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

void ANTNPCCharacterBase::BeginAttack(const EAttackDistance InAttackType)
{
	//MC_LOG(LogMC, Log, TEXT("Begin"));

	CurrentAttack = InAttackType;
	FString EnumString = StaticEnum<EAttackDistance>()->GetNameByValue(static_cast<int64>(CurrentAttack)).ToString();
	//MC_LOG(LogMC, Log, TEXT("CurrentAttack : %s"), *EnumString);

	if (CurrentAttack == EAttackDistance::AttackNone)
	{
		MC_LOG(LogMC, Error, TEXT("No CurrentAttack!"));
		return;
	}

	BTComponent->PauseLogic(TEXT("Start Attack"));

	ACharacter* LocalPlayerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	FVector Direction = LocalPlayerCharacter->GetActorLocation() - GetActorLocation();
	FRotator NewRotation = FRotationMatrix::MakeFromX(Direction).Rotator();
	NewRotation.Pitch = 0.f; NewRotation.Roll = 0.f; // 눕지 않게 고정
	SetActorRotation(NewRotation);

	// Animation Setting
	const float AttackSpeedRate = 1.0f;
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	UAnimMontage* AttackMontage = AttackMontages[CurrentAttack];

	if (AttackMontage == nullptr)
	{
		MC_LOG(LogMC, Error, TEXT("No AttackMontage!"));
		return;
	}

	bIsAttacking = true;

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
	HitActors.Add(OtherActor);

	PlayerCharacter->ApplyDamage(this, AttackDamage, AttackDrain, GuardDrain, AttackParryables[CurrentAttack]);

	//const FVector ImpactPoint = SweepResult.bBlockingHit ? FVector(SweepResult.ImpactPoint) : OtherActor->GetActorLocation();
	//Damageable->ApplyDamage(AttackDamage, this, ImpactPoint, GetActorForwardVector() * 200.0f);
	//UGameplayStatics::ApplyDamage(OtherActor, AttackDamage, GetController(), this, UDamageType::StaticClass());
}

void ANTNPCCharacterBase::OnAttackMontageEnd(UAnimMontage* TargetMontage, bool IsProperlyEnded)
{
	//MC_LOG(LogMC, Log, TEXT("Begin"));

	if (CurrentAttack == EAttackDistance::AttackMiddle)
	{
		// 현재가 연계 공격이거나 횡공격이면
		EndAttack();
		return;
	}

	// 연계 공격 판단하기
	// 거리 계산
	ACharacter* LocalPlayerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	float Dist = FVector::Dist(GetActorLocation(), LocalPlayerCharacter->GetActorLocation());

	EAttackDistance ResultAttckType = EAttackDistance::AttackNone;

	// 근거리
	if (Dist <= AttackRangeShort) // 2.0m
	{
		//MC_LOG(LogMC, Log, TEXT("연계 공격 수행"));

		ResultAttckType = EAttackDistance::AttackMiddle;

		BeginAttack(ResultAttckType);
	}
	else
	{
		EndAttack();
	}

}

void ANTNPCCharacterBase::EndAttack()
{
	bIsAttacking = false;
	CurrentAttack = EAttackDistance::AttackNone;
	BTComponent->ResumeLogic(TEXT("Attack End"));
	OnAICharacterAttackFinished.ExecuteIfBound();
	return;
}

void ANTNPCCharacterBase::OnDamageMontageEnd(UAnimMontage* TargetMontage, bool IsProperlyEnded)
{
	BTComponent->ResumeLogic(TEXT("Damage End"));
}
