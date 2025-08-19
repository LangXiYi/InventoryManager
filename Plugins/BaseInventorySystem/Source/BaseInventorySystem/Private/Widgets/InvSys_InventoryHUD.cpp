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
				DefaultInventoryWidgets.Add(InventorySlot->GetInventoryTag());
				InventoryWidgetMapping.Add(InventorySlot->GetInventoryTag(), InventorySlot);
			}
		});
	}
}

void UInvSys_InventoryHUD::AddWidget(UUserWidget* NewWidget, FGameplayTag InventoryTag)
{
	// 完全匹配标签的控件，直接加入插槽即可
	if (InventoryWidgetMapping.Contains(InventoryTag))
	{
		check(InventoryWidgetMapping[InventoryTag]->HasAnyChildren() == false)
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
			if (LayoutWidget)
			{
				UPanelSlot* PanelSlot = LayoutWidget->AddWidget(NewWidget, InventoryTag);
				if (PanelSlot && PanelSlot->Parent->IsA<UInvSys_InventorySlot>())
				{
					UInvSys_InventorySlot* InventorySlot = Cast<UInvSys_InventorySlot>(PanelSlot->Parent);
					InventoryWidgetMapping.Add(InventoryTag, InventorySlot);
				}
				break;
			}
		}
		ParentInventoryTag = ParentInventoryTag.RequestDirectParent();
	}
}

void UInvSys_InventoryHUD::RemoveWidget(const FGameplayTag& InventoryTag)
{
	if (InventoryWidgetMapping.Contains(InventoryTag))
	{
		UE_LOG(LogInventorySystem, Log, TEXT("Begine Remove Inventory Widget From HUD ==============="))
		UInvSys_InventorySlot* InventorySlot = InventoryWidgetMapping[InventoryTag];
		InventorySlot->ClearChildren();
		TArray<FGameplayTag> RemoveTags;
		for (auto WidgetMapping : InventoryWidgetMapping)
		{
			if (WidgetMapping.Key == InventoryTag)
			{
				// 不会移除自身插槽，仅移除所有子集
				continue;
			}
			if (WidgetMapping.Key.MatchesTag(InventoryTag))
			{
				RemoveTags.Add(WidgetMapping.Key);
			}
		}
		for (const FGameplayTag& RemoveTag : RemoveTags)
		{
			UWidget* InventoryWidget = FindInventoryWidget<UWidget>(RemoveTag);
			if (InventoryWidget)
			{
				InventoryWidget->RemoveFromParent();
				InventoryWidget->ConditionalBeginDestroy();
				InventoryWidget->ConditionalBeginDestroy();
			}
			InventoryWidgetMapping.Remove(RemoveTag);
			UE_LOG(LogInventorySystem, Log, TEXT("\tRemove Inventory Widget %s"), *RemoveTag.ToString())
		}
	}
}

UInvSys_InventorySlot* UInvSys_InventoryHUD::FindInventorySlot(FGameplayTag InventoryTag) const
{
	if (InventoryTag.IsValid() == false)
	{
		return nullptr;
	}
	if (InventoryWidgetMapping.Contains(InventoryTag))
	{
		return InventoryWidgetMapping[InventoryTag];
	}
	return FindInventorySlot(InventoryTag.RequestDirectParent());
}

UUserWidget* UInvSys_InventoryHUD::FindInventoryWidget(FGameplayTag InventoryTag, TSubclassOf<UUserWidget> WidgetClass)
{
	return FindInventoryWidget<UUserWidget>(InventoryTag);
}

UUserWidget* UInvSys_InventoryHUD::FindAndCreateInventoryWidget(FGameplayTag InventoryTag, TSubclassOf<UUserWidget> WidgetClass)
{
	UUserWidget* InventoryWidget = FindInventoryWidget(InventoryTag, WidgetClass);
	if (InventoryWidget == nullptr)
	{
		InventoryWidget = CreateWidget(this, WidgetClass);
		AddWidget(InventoryWidget, InventoryTag);
	}
	check(InventoryWidget);
	return InventoryWidget;
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
