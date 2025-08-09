// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/GridInvSys_InventoryComponent.h"

#include "BaseInventorySystem.h"
#include "Components/InventoryObject/Fragment/GridInvSys_InventoryFragment_Container.h"
#include "Components/InventoryObject/Fragment/InvSys_InventoryFragment_DisplayWidget.h"
#include "Data/GridInvSys_InventoryItemInstance.h"
#include "Data/GridInvSys_ItemFragment_GridItemSize.h"
#include "Data/InvSys_ItemFragment_PickUpItem.h"
#include "Library/GridInvSys_CommonFunctionLibrary.h"
#include "Widgets/GridInvSys_ContainerGridLayoutWidget.h"


// Sets default values for this component's properties
UGridInvSys_InventoryComponent::UGridInvSys_InventoryComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	DefaultContainerPriority.Add(FGameplayTag::RequestGameplayTag("Inventory.Container.Backpack"));
	DefaultContainerPriority.Add(FGameplayTag::RequestGameplayTag("Inventory.Container.ChestRig"));
	DefaultContainerPriority.Add(FGameplayTag::RequestGameplayTag("Inventory.Container.Pocket"));
	DefaultContainerPriority.Add(FGameplayTag::RequestGameplayTag("Inventory.Container.SafeBox"));
}

void UGridInvSys_InventoryComponent::AddItemDefinitionToContainerPos(
	TSubclassOf<UInvSys_InventoryItemDefinition> ItemDef, int32 StackCount, FGridInvSys_ItemPosition Pos)
{
	check(ItemDef)
	if (ItemDef == nullptr)
	{
		return;
	}
	UInvSys_InventoryItemDefinition* ItemDefObj = ItemDef->GetDefaultObject<UInvSys_InventoryItemDefinition>();
	if (ItemDefObj)
	{
		UGridInvSys_InventoryFragment_Container* ContainerFragment =
			FindInventoryObjectFragment<UGridInvSys_InventoryFragment_Container>(Pos.EquipSlotTag);
		if (ContainerFragment)
		{
			FIntPoint ItemSize = UGridInvSys_CommonFunctionLibrary::CalculateItemDefinitionSizeFrom(ItemDef, Pos.Direction);
			if (ContainerFragment->HasEnoughFreeSpace(Pos.Position, Pos.GridID, ItemSize))
			{
				AddItemDefinition<UGridInvSys_InventoryItemInstance>(ItemDef, Pos.EquipSlotTag, StackCount, Pos);
			}
		}
		else
		{
			UE_CLOG(PRINT_INVENTORY_SYSTEM_LOG, LogInventorySystem, Warning, TEXT("无有效的容器片段: %s"), *Pos.ToString())
		}
	}
}

void UGridInvSys_InventoryComponent::AddItemInstanceToContainerPos(UInvSys_InventoryItemInstance* InItemInstance,
	const FGridInvSys_ItemPosition& InPos)
{
	AddItemInstance<UGridInvSys_InventoryItemInstance>(InItemInstance, InPos.EquipSlotTag, InPos);
}

void UGridInvSys_InventoryComponent::RestoreItemInstanceToPos(UInvSys_InventoryItemInstance* InItemInstance,
	const FGridInvSys_ItemPosition& InPos)
{
	check(InItemInstance)
	if (InItemInstance && InItemInstance->IsA<UGridInvSys_InventoryItemInstance>())
	{
		UGridInvSys_InventoryItemInstance* GridItemInstance = Cast<UGridInvSys_InventoryItemInstance>(InItemInstance);
		if (GridItemInstance)
		{
			// todo::改造？
			GridItemInstance->SetItemPosition(InPos);
			bool bIsSuccess = RestoreItemInstance(GridItemInstance);
			if (bIsSuccess == false)
			{
				check(false);
				// todo::丢弃物品至世界？
			}
		}
	}
}

bool UGridInvSys_InventoryComponent::FindEmptyPosition(UInvSys_InventoryItemInstance* InItemInstance, FGridInvSys_ItemPosition& OutPosition)
{
	if (InItemInstance == nullptr)
	{
		return false;
	}
	TArray<FGameplayTag> OutContainerTags;
	auto ContainerPriority = InItemInstance->FindFragmentByClass<UInvSys_ItemFragment_PickUpItem>();
	if (ContainerPriority)
	{
		OutContainerTags = ContainerPriority->ContainerPriority; // 根据各个物品自定义的优先级，优先寻找对应物品
	}
	else
	{
		OutContainerTags = DefaultContainerPriority;
	}
	for (FGameplayTag ContainerTag : OutContainerTags)
	{
		UGridInvSys_InventoryFragment_Container* ContainerFragment =
			FindInventoryObjectFragment<UGridInvSys_InventoryFragment_Container>(ContainerTag);

		if (ContainerFragment)
		{
			if (auto ItemSizeFragment = InItemInstance->FindFragmentByClass<UGridInvSys_ItemFragment_GridItemSize>())
			{
				if (ContainerFragment->FindEmptyPosition(ItemSizeFragment->ItemSize, OutPosition))
				{
					OutPosition.Direction = EGridInvSys_ItemDirection::Horizontal;
					return true;
				}
				if (ContainerFragment->FindEmptyPosition(FIntPoint(ItemSizeFragment->ItemSize.Y, ItemSizeFragment->ItemSize.X), OutPosition))
				{
					OutPosition.Direction = EGridInvSys_ItemDirection::Vertical;
					return true;
				}
			}
		}
	}
	return false;
}

void UGridInvSys_InventoryComponent::UpdateItemInstancePosition(UInvSys_InventoryItemInstance* ItemInstance,
	FGridInvSys_ItemPosition NewPosition)
{

	if (ItemInstance && ItemInstance->IsA<UGridInvSys_InventoryItemInstance>())
	{
		UGridInvSys_InventoryItemInstance* GridItem = Cast<UGridInvSys_InventoryItemInstance>(ItemInstance);
		check(GridItem)
		auto ContainerFragment = FindInventoryObjectFragment<UGridInvSys_InventoryFragment_Container>(NewPosition.EquipSlotTag);
		if (ContainerFragment)
		{
			bool bIsSuccess = ContainerFragment->UpdateItemInstancePosition(GridItem, NewPosition);

			UE_CLOG(PRINT_INVENTORY_SYSTEM_LOG && bIsSuccess == false, LogInventorySystem, Error,
				TEXT("更新 [%s] 位置信息失败 --> {%s}"),
				*ItemInstance->GetItemDisplayName().ToString(),
				*NewPosition.ToString())
		}
	}
}

bool UGridInvSys_InventoryComponent::CheckItemPosition(UInvSys_InventoryItemInstance* ItemInstance,
	const FGridInvSys_ItemPosition& NewPosition) const
{
	if (NewPosition.IsValid() == false)
	{
		UE_LOG(LogInventorySystem, Warning, TEXT("%hs Falied, 传入的位置信息无效."), __FUNCTION__)
		return false;
	}
	if (ItemInstance && ItemInstance->IsA<UGridInvSys_InventoryItemInstance>())
	{
		auto ContainerFragment = FindInventoryObjectFragment<UGridInvSys_InventoryFragment_Container>(NewPosition.EquipSlotTag);
		if (ContainerFragment)
		{
			return ContainerFragment->CheckItemPosition(ItemInstance, NewPosition);
		}
		UE_LOG(LogInventorySystem, Warning, TEXT("%hs Falied, 在 %s 的库存组件中不存在片段 %s."),
			__FUNCTION__, *GetOwner()->GetName(), *NewPosition.EquipSlotTag.ToString())
		return false;
	}
	UE_LOG(LogInventorySystem, Warning, TEXT("%hs Falied, 物品实例为空或类型不为 UGridInvSys_InventoryItemInstance."), __FUNCTION__)
	return false;
}

bool UGridInvSys_InventoryComponent::CancelOccupied(UGridInvSys_InventoryItemInstance* ItemInstance)
{
	if (ItemInstance)
	{
		auto ContainerFragment = FindInventoryObjectFragment<UGridInvSys_InventoryFragment_Container>(ItemInstance->GetInventoryObjectTag());
		if (ContainerFragment)
		{
			ContainerFragment->UpdateContainerGridItemState(ItemInstance, ItemInstance->GetItemPosition(), false);
			return true;
		}
	}
	return false;
}

UGridInvSys_ContainerGridWidget* UGridInvSys_InventoryComponent::FindContainerGridWidget(FGameplayTag SlotTag,
                                                                                         int32 GridID)
{
	UInvSys_InventoryFragment_DisplayWidget* DisplayFragment =
		FindInventoryObjectFragment<UInvSys_InventoryFragment_DisplayWidget>(SlotTag);
	if (DisplayFragment)
	{
		UGridInvSys_ContainerGridLayoutWidget* ContainerLayoutWidget =
			DisplayFragment->GetDisplayWidget<UGridInvSys_ContainerGridLayoutWidget>();

		if (ContainerLayoutWidget)
		{
			return ContainerLayoutWidget->FindContainerGrid(GridID);
		}
	}
	return nullptr;
}

UGridInvSys_ContainerGridWidget* UGridInvSys_InventoryComponent::FindContainerGridWidget(
	UGridInvSys_InventoryItemInstance* InItemInstance)
{
	check(InItemInstance)
	return FindContainerGridWidget(InItemInstance->GetInventoryObjectTag(), InItemInstance->GetItemPosition().GridID);
}