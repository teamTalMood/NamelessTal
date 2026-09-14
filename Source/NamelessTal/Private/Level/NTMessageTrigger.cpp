// Fill out your copyright notice in the Description page of Project Settings.


#include "Level/NTMessageTrigger.h"

#include "Components/BoxComponent.h"
#include "Player/TalPlayerCharacter.h"

// Sets default values
ANTMessageTrigger::ANTMessageTrigger()
{
 	PrimaryActorTick.bCanEverTick = false;

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	RootComponent = TriggerBox;

	TriggerBox->SetBoxExtent(FVector(100.0f, 100.0f, 100.0f));
	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	TriggerBox->OnComponentBeginOverlap.AddDynamic(
		this,&ANTMessageTrigger::OnTriggerBeginOverlap);
}

void ANTMessageTrigger::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ATalPlayerCharacter* Player = Cast<ATalPlayerCharacter>(OtherActor);
	if (Player == nullptr)
	{
		return;
	}
	ATalPlayerController* PlayerController = 
		Cast<ATalPlayerController>(Player->GetController());
	if (PlayerController == nullptr)
	{
		return;
	}

	PlayerController->ShowMessage(Style, Title, Body, Duration);

	if (bOnce)
	{
		TriggerBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

}

