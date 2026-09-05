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

//AI
#include "BehaviorTree/BehaviorTreeComponent.h"

//NT
#include "NPC/NTAIController.h"
#include "Animation/AnimMontage.h"
#include "Stat/StatComponent.h"

//Interface
#include "Variant_Combat/Interfaces/CombatDamageable.h"

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

	// Attack Direction Set
	AttackDirections.Add(EAttackDistance::AttackNone, 0);
	AttackDirections.Add(EAttackDistance::AttackShort, 0);
	AttackDirections.Add(EAttackDistance::AttackMiddle, 0);
	AttackDirections.Add(EAttackDistance::AttackLong, 0);
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
}

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

void ANTNPCCharacterBase::AttackHitCheck()
{
	MC_LOG(LogMC, Log, TEXT("Begin"));

	FHitResult OutHitResult;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(Attack), false, this);

	const float AttackRange = 40.0f;
	const float AttackRadius = 50.0f;
	const FVector Start = GetActorLocation() + GetActorForwardVector() * GetCapsuleComponent()->GetScaledCapsuleRadius();
	const FVector End = Start + GetActorForwardVector() * AttackRange;

	bool HitDetected = GetWorld()->SweepSingleByChannel(OutHitResult, Start, End, FQuat::Identity, ECC_WorldDynamic, FCollisionShape::MakeSphere(AttackRadius), Params);
	if (HitDetected)
	{
		AActor* HitActor = OutHitResult.GetActor();
		if (HitActor)
		{
			UGameplayStatics::ApplyDamage(HitActor, AttackDamage, GetController(), this, UDamageType::StaticClass());
		}
	}
	MC_LOG(LogMC, Log, TEXT("HitDetected : %d"), HitDetected);

#if ENABLE_DRAW_DEBUG

	FVector CapsuleOrigin = Start + (End - Start) * 0.5f;
	float CapsuleHalfHeight = AttackRange * 0.5f;
	FColor DrawColor = HitDetected ? FColor::Green : FColor::Red;

	DrawDebugCapsule(GetWorld(), CapsuleOrigin, CapsuleHalfHeight, AttackRadius, FRotationMatrix::MakeFromZ(GetActorForwardVector()).ToQuat(), DrawColor, false, 5.0f);

#endif

	OnAICharacterAttackFinished.ExecuteIfBound();

	MC_LOG(LogMC, Log, TEXT("End"));
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

	//ICombatDamageable* Damageable = Cast<ICombatDamageable>(OtherActor);
	//if (Damageable == nullptr)
	//{
	//	return;
	//}
	HitActors.Add(OtherActor);

	//const FVector ImpactPoint = SweepResult.bBlockingHit ? FVector(SweepResult.ImpactPoint) : OtherActor->GetActorLocation();
	//Damageable->ApplyDamage(AttackDamage, this, ImpactPoint, GetActorForwardVector() * 200.0f);
	UGameplayStatics::ApplyDamage(OtherActor, AttackDamage, GetController(), this, UDamageType::StaticClass());
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
