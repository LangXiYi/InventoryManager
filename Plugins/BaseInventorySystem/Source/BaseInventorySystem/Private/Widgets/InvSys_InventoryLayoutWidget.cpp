// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/InvSys_InventoryLayoutWidget.h"

#include "BaseInventorySystem.h"
#include "Blueprint/WidgetTree.h"
#include "Widgets/Components/InvSys_InventorySlot.h"


UInvSys_InventorySlot* UInvSys_InventoryLayoutWidget::FindTagSlot(FGameplayTag InSlotTag)
{
	if (InventoryWidgetMapping.Contains(InSlotTag))
	{
		return InventoryWidgetMapping[InSlotTag];
	}
	return nullptr;
}

void UInvSys_InventoryLayoutWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	if (WidgetTree)
	{
		WidgetTree->ForEachWidget([&] (UWidget* Widget) {
			if (Widget->IsA(UInvSys_InventorySlot::StaticClass()))
			{
				UInvSys_InventorySlot* TagSlot = Cast<UInvSys_InventorySlot>(Widget);
				check(TagSlot)
				InventoryWidgetMapping.Emplace(TagSlot->GetInventoryTag(), TagSlot);
			}
		});
	}
}

UPanelSlot* UInvSys_InventoryLayoutWidget::AddWidget(UUserWidget* Widget, const FGameplayTag& Tag)
{
	// 根据库存对象的标签查询对应的槽位，然后将需要显示的控件添加到目标槽位下。
	if (UInvSys_InventorySlot* TagSlot = FindTagSlot(Tag))
	{
		return TagSlot->AddChild(Widget);
	}
	else
	{
		UE_LOG(LogInventorySystem, Error, TEXT("标签为 %s 的控件在布局中未找到"), *Tag.ToString());
	}
	return nullptr;
}
