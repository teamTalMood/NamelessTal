#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NTAreaNoticeWidget.generated.h"

class UTextBlock;
class UImage;
class UTexture2D;

/**
 * 지역에 도착했을 때 뜨는 알림.
 * 트리거 안에 머무는 동안 계속 떠 있고, 나가면 등장 애니메이션을 역재생하며 사라진다.
 * 일정 시간 뒤 저절로 사라지는 NTMessageWidget과는 별개의 위젯이다.
 */
UCLASS()
class NAMELESSTAL_API UNTAreaNoticeWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 트리거 진입 시 호출. 시간이 지나도 저절로 사라지지 않는다.
	void ShowArea(const FText& AreaName, const FText& Description, UTexture2D* Icon);

	// 트리거 이탈 시 호출. 역재생이 끝나면 숨긴다.
	void HideArea();

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_AreaName;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_Description;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Img_Icon;

	// 등장 애니메이션. 퇴장은 이것을 역재생한다.
	UPROPERTY(meta = (BindWidgetAnim), Transient)
	TObjectPtr<UWidgetAnimation> Anim_Show;

private:
	FTimerHandle CollapseTimer;
};