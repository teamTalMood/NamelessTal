// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NTCutsceneWidget.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnCutsceneSkipRequested);

/**
 * 렌더 시퀀스 재생용 위젯. 영상 표시는 위젯 블루프린트에서 Image 브러시를 MediaTexture로 지정한다.
 * 스페이스/ESC 입력을 감지해 스킵 요청만 알린다.
 */
UCLASS()
class NAMELESSTAL_API UNTCutsceneWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	FOnCutsceneSkipRequested OnSkipRequested;

protected:
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
};
