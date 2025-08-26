// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/InvSys_InventoryLayoutWidget.h"

#include "BaseInventorySystem.h"
#include "Blueprint/WidgetTree.h"
#include "Widgets/Components/InvSys_InventoryWidgetSlot.h"


UInvSys_InventoryWidgetSlot* UInvSys_InventoryLayoutWidget::FindTagSlot(FGameplayTag InSlotTag)
{
	if (InventoryWidgetMapping.Contains(InSlotTag))
	{
		return InventoryWidgetMapping[InSlotTag];
	}
	return nullptr;
}

TSharedRef<SWidget> UInvSys_InventoryLayoutWidget::RebuildWidget()
{
	if (WidgetTree)
	{
		WidgetTree->ForEachWidget([&] (UWidget* Widget) {
			if (Widget->IsA(UInvSys_InventoryWidgetSlot::StaticClass()))
			{
				UInvSys_InventoryWidgetSlot* TagSlot = Cast<UInvSys_InventoryWidgetSlot>(Widget);
				check(TagSlot)
				InventoryWidgetMapping.Emplace(TagSlot->GetInventoryTag(), TagSlot);
			}
		});
	}
	return Super::RebuildWidget();
}

UInvSys_InventoryWidgetSlot* UInvSys_InventoryLayoutWidget::AddWidget(UUserWidget* Widget, const FGameplayTag& Tag)
{
	// 根据库存对象的标签查询对应的槽位，然后将需要显示的控件添加到目标槽位下。
	if (UInvSys_InventoryWidgetSlot* TagSlot = FindTagSlot(Tag))
	{
		TagSlot->AddChild(Widget);
		return TagSlot;
	}
	else
	{
		UE_LOG(LogInventorySystem, Error, TEXT("标签为 %s 的控件在布局中未找到"), *Tag.ToString());
	}
	return nullptr;
}
