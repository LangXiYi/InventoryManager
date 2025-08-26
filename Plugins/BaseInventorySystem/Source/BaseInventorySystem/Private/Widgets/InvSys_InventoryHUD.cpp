// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/InvSys_InventoryHUD.h"

#include "BaseInventorySystem.h"
#include "InvSys_InventorySystemConfig.h"
#include "Blueprint/WidgetBlueprintGeneratedClass.h"
#include "Blueprint/WidgetTree.h"
#include "Widgets/InvSys_InventoryLayoutWidget.h"
#include "Widgets/Components/InvSys_InventoryItemActionPanel.h"
#include "Widgets/Components/InvSys_InventoryWidgetSlot.h"

UInvSys_InventoryHUD::UInvSys_InventoryHUD(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	// UWidgetBlueprintGeneratedClass* BGClass = Cast<UWidgetBlueprintGeneratedClass>(GetClass());
	// BGClass->bCanCallInitializedWithoutPlayerContext = true;
}

TSharedRef<SWidget> UInvSys_InventoryHUD::RebuildWidget()
{
	if (WidgetTree)
	{
		WidgetTree->ForEachWidget([&] (UWidget* Widget) {
			if (Widget->IsA<UInvSys_InventoryWidgetSlot>())
			{
				UInvSys_InventoryWidgetSlot* InventorySlot = Cast<UInvSys_InventoryWidgetSlot>(Widget);
				DefaultInventoryWidgets.Add(InventorySlot->GetInventoryTag());
				InventoryWidgetMapping.Add(InventorySlot->GetInventoryTag(), InventorySlot);
			}
		});
	}
	return Super::RebuildWidget();
}

bool UInvSys_InventoryHUD::AddWidget(UUserWidget* NewWidget, FGameplayTag InventoryTag)
{
	// todo::根据生命周期更新控件
	// 完全匹配标签的控件，直接加入插槽即可
	if (InventoryWidgetMapping.Contains(InventoryTag))
	{
		if (InventoryWidgetMapping[InventoryTag]->HasAnyChildren())
		{
			return false;
		}
		InventoryWidgetMapping[InventoryTag]->AddInventorySlotChild(NewWidget);
		return true;
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
				UInvSys_InventoryWidgetSlot* InventorySlot = LayoutWidget->AddWidget(NewWidget, InventoryTag);
				if (InventorySlot)
				{
					InventoryWidgetMapping.Add(InventoryTag, InventorySlot);
					return true;
				}
				// 及时退出，不再继续向后查找
				break;
			}
		}
		ParentInventoryTag = ParentInventoryTag.RequestDirectParent();
	}
	return false;
}

void UInvSys_InventoryHUD::RemoveWidget(FGameplayTag InventoryTag)
{
	if (InventoryWidgetMapping.Contains(InventoryTag))
	{
		UE_LOG(LogInventorySystem, Log, TEXT("Begine Remove Inventory Widget From HUD ==============="))
		TArray<FGameplayTag> RemoveTags;
		for (auto WidgetMapping : InventoryWidgetMapping)
		{
			if (WidgetMapping.Key.MatchesTag(InventoryTag))
			{
				RemoveTags.Add(WidgetMapping.Key);
			}
		}
		for (const FGameplayTag& RemoveTag : RemoveTags)
		{
			UInvSys_InventoryWidgetSlot* InventorySlot = FindInventorySlot(RemoveTag);
			if (InventorySlot)
			{
				UUserWidget* SlotChild = InventorySlot->GetInventorySlotChild();
				if (SlotChild)
				{
					if (SlotChild->IsA<UInvSys_InventoryLayoutWidget>())
					{
						// 不移除控件布局
						continue;
					}
					InventorySlot->ClearChildren();
					SlotChild->ConditionalBeginDestroy();
				}
			}
			// 不会移除自身插槽，仅移除所有子集及其插槽
			if (RemoveTag != InventoryTag)
			{
				InventoryWidgetMapping.Remove(RemoveTag);
			}
			UE_LOG(LogInventorySystem, Log, TEXT("\tRemove Inventory Widget %s"), *RemoveTag.ToString())
		}
	}
}

void UInvSys_InventoryHUD::SetInventoryWidgetVisibility(FGameplayTag InventoryTag, ESlateVisibility InVisibility)
{
	UInvSys_InventoryWidgetSlot* InventorySlot = FindInventorySlot(InventoryTag);
	if (InventorySlot && InventorySlot->HasAnyChildren())
	{
		UUserWidget* ChildWidget = InventorySlot->GetInventorySlotChild();
		if (ChildWidget)
		{
			ChildWidget->SetVisibility(InVisibility);
		}
	}
}

void UInvSys_InventoryHUD::SetInventorySlotVisibility(FGameplayTag InventoryTag, ESlateVisibility InVisibility)
{
	UInvSys_InventoryWidgetSlot* InventorySlot = FindInventorySlot(InventoryTag);
	if (InventorySlot)
	{
		InventorySlot->SetVisibility(InVisibility);
	}
}

UInvSys_InventoryWidgetSlot* UInvSys_InventoryHUD::FindInventorySlot(FGameplayTag InventoryTag) const
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