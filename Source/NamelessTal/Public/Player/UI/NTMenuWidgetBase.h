#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NTMenuWidgetBase.generated.h"

class UWidget;

/**
 * 셀렉터(낙관+밑줄)가 항목을 따라다니는 메뉴들의 공통 부모.
 * 게임을 멈추고 입력을 UI로 돌리는 처리까지 여기서 담당한다.
 */
UCLASS(Abstract)
class NAMELESSTAL_API UNTMenuWidgetBase : public UUserWidget
{
	GENERATED_BODY()


public:
	UNTMenuWidgetBase(const FObjectInitializer& ObjectInitializer);

public:
	// 뷰포트에 올리고 게임을 멈춘다.
	void OpenMenu();

	// 게임을 재개하고 뷰포트에서 내린다. ESC와 버튼 양쪽에서 호출한다.
	virtual void CloseMenu();

protected:
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

	// 셀렉터를 해당 위젯 자리로 옮긴다. 각 메뉴가 호버 핸들러에서 호출한다.
	void MoveSelectorTo(UWidget* Target);

	// 낙관 + 밑줄을 담은 컨테이너. 이 위치만 옮긴다.
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidget> Selector;

	// 버튼 기준 셀렉터 오프셋. 디자인 보면서 맞춘다.
	UPROPERTY(EditAnywhere, Category = "Menu")
	FVector2D SelectorOffset = FVector2D(0.0f, 46.0f);
};