// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/GridInvSys_InventoryComponent.h"

#include "BaseInventorySystem.h"
#include "Components/InventoryObject/Fragment/GridInvSys_InventoryFragment_Container.h"
#include "Components/InventoryObject/Fragment/InvSys_InventoryFragment_DisplayWidget.h"
#include "Data/GridInvSys_InventoryItemInstance.h"
#include "Data/GridInvSys_ItemFragment_GridItemSize.h"
#include "Data/InvSys_ItemFragment_ContainerPriority.h"
#include "Library/GridInvSys_CommonFunctionLibrary.h"
#include "Widgets/GridInvSys_ContainerGridLayoutWidget.h"


// Sets default values for this component's properties
UGridInvSys_InventoryComponent::UGridInvSys_InventoryComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
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
	UGridInvSys_InventoryItemInstance* GridItemInstance = Cast<UGridInvSys_InventoryItemInstance>(InItemInstance);
	check(GridItemInstance)
	if (GridItemInstance)
	{
		UGridInvSys_InventoryFragment_Container* ContainerFragment =
			FindInventoryObjectFragment<UGridInvSys_InventoryFragment_Container>(InPos.EquipSlotTag);
		if (ContainerFragment)
		{
			FIntPoint ItemSize = UGridInvSys_CommonFunctionLibrary::CalculateItemInstanceSizeFrom(InItemInstance, InPos.Direction);
			if (ContainerFragment->HasEnoughFreeSpace(InPos.Position, InPos.GridID, ItemSize))
			{
				AddItemInstance<UGridInvSys_InventoryItemInstance>(GridItemInstance, InPos.EquipSlotTag, InPos);
			}
		}
	}
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
	auto ContainerPriority = InItemInstance->FindFragmentByClass<UInvSys_ItemFragment_ContainerPriority>();
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
	return FindContainerGridWidget(InItemInstance->GetSlotTag(), InItemInstance->GetItemPosition().GridID);
}