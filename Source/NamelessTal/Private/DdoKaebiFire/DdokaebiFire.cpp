// Fill out your copyright notice in the Description page of Project Settings.


#include "DdoKaebiFire/DdokaebiFire.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"

#include "InputMappingContext.h"
#include "InputAction.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Engine/LocalPlayer.h"


// Sets default values
ADdokaebiFire::ADdokaebiFire()
{
	//틱사용 안함
	PrimaryActorTick.bCanEverTick = false;

	//Root Mesh = FireMesh로 
	FireMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FireMesh"));
	RootComponent = FireMesh;

	FireInteractionRange = CreateDefaultSubobject<USphereComponent>(TEXT("FireInteractionRange"));
	FireInteractionRange->SetupAttachment(RootComponent);
	FireInteractionRange->SetSphereRadius(200.0f);
	FireInteractionRange->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	FireInteractionRange->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);

	FireInteractionRange->OnComponentBeginOverlap.AddDynamic(this,
		&ADdokaebiFire::OnRangeBeginOverlap);
	FireInteractionRange->OnComponentEndOverlap.AddDynamic(this,
		&ADdokaebiFire::OnRangeEndOverlap);
}

// Called when the game starts or when spawned
void ADdokaebiFire::BeginPlay()
{
	Super::BeginPlay();
	
}

void ADdokaebiFire::OnRangeBeginOverlap(UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	UE_LOG(LogTemp, Warning, TEXT("범위 진입 감지! OtherActor: %s"), *GetNameSafe(OtherActor));
	APawn* Pawn = Cast<APawn>(OtherActor);
	if (Pawn == nullptr)
	{
		UE_LOG(LogTemp, Log, TEXT("Pawn Cast Failed"));
		return;
	}
	APlayerController* PC = Cast<APlayerController>(Pawn->GetController());
	if (PC == nullptr)
	{
		UE_LOG(LogTemp, Log, TEXT("PC Cast Failed"));
		return;
	}

	OverlappingPlayer = OtherActor;

	EnableInput(PC);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>
		(InputComponent))
	{
		//중복 바인딩 방지
		EnhancedInputComponent->ClearActionBindings();
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started
			, this, &ADdokaebiFire::OnInteract);
	}

	if (UEnhancedInputLocalPlayerSubsystem* SubSystem = ULocalPlayer::GetSubsystem
		<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
	{
		SubSystem->AddMappingContext(InteractMappingContext, 1);
	}

}

void ADdokaebiFire::OnRangeEndOverlap(UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{

	UE_LOG(LogTemp, Warning, TEXT("범위 나감 감지! OtherActor: %s"), *GetNameSafe(OtherActor));
	if (OtherActor == OverlappingPlayer)
	{
		APawn* Pawn = Cast<APawn>(OtherActor);
		APlayerController* PC = Pawn ? Cast<APlayerController>(Pawn->GetController()) : nullptr;

		if (PC != nullptr)
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
				ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>
				(PC->GetLocalPlayer()))
			{
				Subsystem->RemoveMappingContext(InteractMappingContext);
			}
			DisableInput(PC);
		}
		OverlappingPlayer = nullptr;
	}

}

void ADdokaebiFire::OnInteract()
{
	// 나중에 HP, 넋 시스템 연결
	UE_LOG(LogTemp, Warning, TEXT("모닥불 기사 등장! : %s"), *GetNameSafe(OverlappingPlayer));

}
