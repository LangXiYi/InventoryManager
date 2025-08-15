// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/InvSys_InventoryHUD.h"

#include "BaseInventorySystem.h"
#include "Blueprint/WidgetTree.h"
#include "Widgets/InvSys_InventoryLayoutWidget.h"
#include "Widgets/Components/InvSys_InventoryItemActionPanel.h"
#include "Widgets/Components/InvSys_InventorySlot.h"

void UInvSys_InventoryHUD::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	if (WidgetTree)
	{
		WidgetTree->ForEachWidget([&] (UWidget* Widget) {
			if (Widget->IsA<UInvSys_InventorySlot>())
			{
				UInvSys_InventorySlot* InventorySlot = Cast<UInvSys_InventorySlot>(Widget);
				InventoryWidgetMapping.Add(InventorySlot->GetInventoryTag(), InventorySlot);
			}
		});
	}
}

void UInvSys_InventoryHUD::AddWidget(UUserWidget* NewWidget, const FGameplayTag& InventoryTag)
{
	// 完全匹配标签的控件，直接加入插槽即可
	if (InventoryWidgetMapping.Contains(InventoryTag))
	{
		InventoryWidgetMapping[InventoryTag]->AddInventorySlotChild(NewWidget);
		return;
	}

	// 不完全匹配标签的控件，需要遍历父级
	FGameplayTag ParentInventoryTag = InventoryTag;
	while (ParentInventoryTag.IsValid())
	{
		if (InventoryWidgetMapping.Contains(ParentInventoryTag))
		{
			auto LayoutWidget = InventoryWidgetMapping[ParentInventoryTag]->GetInventorySlotChild<UInvSys_InventoryLayoutWidget>();
			checkf(LayoutWidget, TEXT("布局不存在，却为其添加子控件。"));
			UPanelSlot* PanelSlot = LayoutWidget->AddWidget(NewWidget, InventoryTag);
			if (PanelSlot && PanelSlot->Parent->IsA<UInvSys_InventorySlot>())
			{
				UInvSys_InventorySlot* InventorySlot = Cast<UInvSys_InventorySlot>(PanelSlot->Parent);
				InventoryWidgetMapping.Add(InventoryTag, InventorySlot);
			}
			break;
		}
		ParentInventoryTag = ParentInventoryTag.RequestDirectParent();
	}
}

UWidget* UInvSys_InventoryHUD::FindInventoryWidget(FGameplayTag InventoryTag, TSubclassOf<UWidget> WidgetClass)
{
	return FindInventoryWidget<UWidget>(InventoryTag);
}

void UInvSys_InventoryHUD::DisplayInventoryItemActionList(UInvSys_InventoryItemInstance* ItemInstance)
{
	if (ItemInstance == nullptr)
	{
		UE_LOG(LogInventorySystem, Error, TEXT("%hs Falied, ItemInstance is nullptr."), __FUNCTION__)
		return;
	}
	if (ItemActionPanel == nullptr)
	{
		ItemActionPanel = CreateWidget<UInvSys_InventoryItemActionPanel>(this, ItemActionPanelClass);
		ItemActionPanel->AddToViewport();
	}
	check(ItemActionPanel);
	ItemActionPanel->NativeOnCallOut(ItemInstance);
}
