// Fill out your copyright notice in the Description page of Project Settings.


#include "Gimmick/NTGate.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "MCLOG.h"

#include "Interface/NTGateInteractInterface.h"

// Sets default values
ANTGate::ANTGate()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	BoxCollisionComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollision"));
	RootComponent = BoxCollisionComponent;

	GateFrameMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GateFrameMesh"));
	GateFrameMeshComponent->SetupAttachment(RootComponent);

	GateHingeLComponent = CreateDefaultSubobject<USceneComponent>(TEXT("GateHingeL"));
	GateHingeLComponent->SetupAttachment(GateFrameMeshComponent, TEXT("Socket_Door_L"));

	GateHingeRComponent = CreateDefaultSubobject<USceneComponent>(TEXT("GateHingeR"));
	GateHingeRComponent->SetupAttachment(GateFrameMeshComponent, TEXT("Socket_Door_R"));

	GateMeshLComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GateMeshL"));
	GateMeshLComponent->SetupAttachment(GateHingeLComponent);

	GateMeshRComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GateMeshR"));
	GateMeshRComponent->SetupAttachment(GateHingeRComponent);

	PlayerInteractPointComponent = CreateDefaultSubobject<USceneComponent>(TEXT("PlayerInteractPoint"));
	PlayerInteractPointComponent->SetupAttachment(GateFrameMeshComponent, TEXT("Socket_PlayerInteract"));

	BoxCollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &ANTGate::OnBoxCollisionBeginOverlap);
	BoxCollisionComponent->OnComponentEndOverlap.AddDynamic(this, &ANTGate::OnBoxCollisionEndOverlap);
}

// Called when the game starts or when spawned
void ANTGate::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ANTGate::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bIsDoorMoving)
	{
		return;
	}

	MC_LOG(LogMC, Log, TEXT("DoorOpening"));

	DoorOpenElapsedTime += DeltaTime;
	const float Alpha = FMath::Clamp(DoorOpenElapsedTime / DoorOpenDuration, 0.0f, 1.0f);

	GateHingeLComponent->SetRelativeRotation(FMath::Lerp(GateMeshLStartRotation, GateMeshLTargetRotation, Alpha));
	GateHingeRComponent->SetRelativeRotation(FMath::Lerp(GateMeshRStartRotation, GateMeshRTargetRotation, Alpha));

	if (Alpha >= 1.0f)
	{
		bIsDoorMoving = false;
		bIsDoorOpened ^= true;
	}
}

void ANTGate::OnBoxCollisionBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	MC_LOG(LogMC, Log, TEXT("NTGate 콜리전 오버랩 감지"));

	if (bIsDoorMoving || bIsDoorOpened)
	{
		return;
	}

	RequestOpen(OtherActor);

	bIsDoorMoving = true;
	DoorOpenElapsedTime = 0.0f;

	GateMeshLStartRotation = GateHingeLComponent->GetRelativeRotation();
	GateMeshRStartRotation = GateHingeRComponent->GetRelativeRotation();

	if (bGateOpenFront)
	{
		GateMeshLTargetRotation = GateMeshLStartRotation + FRotator(0.0f, 90.0f, 0.0f);
		GateMeshRTargetRotation = GateMeshRStartRotation + FRotator(0.0f, -90.0f, 0.0f);
	}
	else
	{
		GateMeshLTargetRotation = GateMeshLStartRotation + FRotator(0.0f, -90.0f, 0.0f);
		GateMeshRTargetRotation = GateMeshRStartRotation + FRotator(0.0f, 90.0f, 0.0f);
	}
}

void ANTGate::OnBoxCollisionEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	MC_LOG(LogMC, Log, TEXT("NTGate 콜리전 오버랩 벗어남 감지"));

	if (bIsDoorMoving || !bIsDoorOpened)
	{
		return;
	}

	bIsDoorMoving = true;
	DoorOpenElapsedTime = 0.0f;

	GateMeshLStartRotation = GateHingeLComponent->GetRelativeRotation();
	GateMeshRStartRotation = GateHingeRComponent->GetRelativeRotation();

	if (bGateOpenFront)
	{
		GateMeshLTargetRotation = GateMeshLStartRotation + FRotator(0.0f, -90.0f, 0.0f);
		GateMeshRTargetRotation = GateMeshRStartRotation + FRotator(0.0f, 90.0f, 0.0f);
	}
	else
	{
		GateMeshLTargetRotation = GateMeshLStartRotation + FRotator(0.0f, 90.0f, 0.0f);
		GateMeshRTargetRotation = GateMeshRStartRotation + FRotator(0.0f, -90.0f, 0.0f);
	}
}

void ANTGate::RequestOpen(AActor* Player)
{
	if (!IsValid(Player) || !PlayerInteractPointComponent)
	{
		return;
	}

	INTGateInteractInterface* Interactable = Cast<INTGateInteractInterface>(Player);
	if (!Interactable)
	{
		MC_LOG(LogMC, Warning, TEXT("플레이어가 NTGateInteractInterface를 구현하지 않음"));
		return;
	}

	Interactable->PlayGateOpenMontage(PlayerInteractPointComponent->GetComponentTransform(), GateOpenMontage);
}

