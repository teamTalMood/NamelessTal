// Fill out your copyright notice in the Description page of Project Settings.


#include "NPC/Components/NTHPWidgetComponent.h"
#include "MCLOG.h"
#include "NPC/Components/NTHPBarWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/PlayerCameraManager.h"

void UNTHPWidgetComponent::InitWidget()
{
	Super::InitWidget();

	MC_LOG(LogMC, Log, TEXT("Begin"));

	UNTHPBarWidget* UserWidget = Cast<UNTHPBarWidget>(GetWidget());
	if (UserWidget)
	{
		UserWidget->SetOwiningActor(GetOwner());
	}
}

void UNTHPWidgetComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	APlayerCameraManager* CameraManager = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0);
	if (CameraManager == nullptr)
	{
		return;
	}

	const FVector ToCamera = CameraManager->GetCameraLocation() - GetComponentLocation();
	const FRotator FaceCameraRotation = FRotationMatrix::MakeFromX(ToCamera).Rotator();

	SetWorldRotation(FRotator(0.0f, FaceCameraRotation.Yaw, 0.0f));
}
