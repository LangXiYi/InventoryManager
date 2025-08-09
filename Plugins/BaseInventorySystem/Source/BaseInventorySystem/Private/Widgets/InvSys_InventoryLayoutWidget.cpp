// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/InvSys_InventoryLayoutWidget.h"

#include "BaseInventorySystem.h"
#include "Blueprint/WidgetTree.h"
#include "Widgets/Components/InvSys_TagSlot.h"


UInvSys_TagSlot* UInvSys_InventoryLayoutWidget::FindTagSlot(FGameplayTag InSlotTag)
{
	if (TagSlots.Contains(InSlotTag))
	{
		return TagSlots[InSlotTag];
	}
	return nullptr;
}

void UInvSys_InventoryLayoutWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	if (WidgetTree)
	{
		WidgetTree->ForEachWidget([&] (UWidget* Widget) {
			if (Widget->IsA(UInvSys_TagSlot::StaticClass()))
			{
				UInvSys_TagSlot* TagSlot = Cast<UInvSys_TagSlot>(Widget);
				check(TagSlot)
				TagSlots.Emplace(TagSlot->GetSlotTag(), TagSlot);
			}
		});
	}
}

void UInvSys_InventoryLayoutWidget::AddWidget(UUserWidget* Widget, const FGameplayTag& Tag)
{
	// 根据库存对象的标签查询对应的槽位，然后将需要显示的控件添加到目标槽位下。
	if (UInvSys_TagSlot* TagSlot = FindTagSlot(Tag))
	{
		TagSlot->AddChild(Widget);
	}
	else
	{
		UE_LOG(LogInventorySystem, Error, TEXT("标签为 %s 的控件在布局中未找到"), *Tag.ToString());
	}
}
