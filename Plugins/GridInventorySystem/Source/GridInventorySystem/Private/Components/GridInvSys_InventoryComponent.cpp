// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/GridInvSys_InventoryComponent.h"

#include "BaseInventorySystem.h"
#include "Components/InventoryObject/Fragment/GridInvSys_InventoryModule_Container.h"
#include "Components/InventoryObject/Fragment/InvSys_InventoryModule_Display.h"
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
	if (IsValidInventoryTag(Pos.EquipSlotTag) == false)
	{
		UE_CLOG(PRINT_INVENTORY_SYSTEM_LOG, LogInventorySystem, Warning, TEXT("%hs Falied, EquipSlotTag Is not valid %s"), __FUNCTION__, *Pos.EquipSlotTag.ToString())
		return;
	}
	UGridInvSys_InventoryModule_Container* ContainerFragment =
		FindInventoryModule<UGridInvSys_InventoryModule_Container>(Pos.EquipSlotTag);
	if (ContainerFragment == nullptr)
	{
		UE_CLOG(PRINT_INVENTORY_SYSTEM_LOG, LogInventorySystem, Warning, TEXT("无有效的容器片段: %s"), *Pos.ToString())
		return;
	}

	FIntPoint ItemSize = UGridInvSys_CommonFunctionLibrary::CalculateItemDefinitionSizeFrom(ItemDef, Pos.Direction);
	if (ContainerFragment->IsUnoccupiedInSquareRange(Pos.GridID, Pos.Position, ItemSize))
	{
		AddItemDefinition<UGridInvSys_InventoryItemInstance>(ItemDef, Pos.EquipSlotTag, StackCount, Pos);
	}
}

void UGridInvSys_InventoryComponent::AddItemInstanceToContainerPos(UInvSys_InventoryItemInstance* InItemInstance,
	const FGridInvSys_ItemPosition& InPos)
{
	AddItemInstance<UGridInvSys_InventoryItemInstance>(InItemInstance, InPos.EquipSlotTag, InPos);
}

bool UGridInvSys_InventoryComponent::FindEmptyPosition(UInvSys_InventoryItemInstance* InItemInstance, FGridInvSys_ItemPosition& OutPosition)
{
	if (InItemInstance == nullptr)
	{
		UE_LOG(LogInventorySystem, Error, TEXT("%hs Falied, ItemInstance is nullptr."), __FUNCTION__)
		return false;
	}
	return FindEmptyPosition(InItemInstance->GetItemDefinition(), OutPosition);
}

bool UGridInvSys_InventoryComponent::FindEmptyPosition(TSubclassOf<UInvSys_InventoryItemDefinition> ItemDefinition,
	FGridInvSys_ItemPosition& OutPosition)
{
	OutPosition = FindAvailablePosition(ItemDefinition);
	return OutPosition.IsValid();
}

FGridInvSys_ItemPosition UGridInvSys_InventoryComponent::FindAvailablePosition(
	TSubclassOf<UInvSys_InventoryItemDefinition> ItemDefinition, const TArray<FGameplayTag>& Ignores)
{
	FGridInvSys_ItemPosition Result;
	UInvSys_InventoryItemDefinition* CDO_ItemDefinition = ItemDefinition.GetDefaultObject();
	if (CDO_ItemDefinition == nullptr)
	{
		UE_LOG(LogInventorySystem, Error, TEXT("%hs Falied, CDO_ItemDefinition is nullptr."), __FUNCTION__)
		return Result;
	}
	TArray<FGameplayTag> OutContainerTags;
	auto ContainerPriority = CDO_ItemDefinition->FindFragmentByClass<UInvSys_ItemFragment_PickUpItem>();
	if (ContainerPriority && ContainerPriority->ContainerPriority.Num() > 0)
	{
		OutContainerTags = ContainerPriority->ContainerPriority;
	}
	else
	{
		OutContainerTags = DefaultContainerPriority;
	}
	check(OutContainerTags.IsEmpty() == false);

	// 根据各个物品自定义的优先级寻找容器
	for (FGameplayTag ContainerTag : OutContainerTags)
	{
		if (Ignores.Contains(ContainerTag))
		{
			continue;
		}
		UGridInvSys_InventoryModule_Container* ContainerFragment =
			FindInventoryModule<UGridInvSys_InventoryModule_Container>(ContainerTag);

		if (ContainerFragment)
		{
			if (auto ItemSizeFragment = CDO_ItemDefinition->FindFragmentByClass<UGridInvSys_ItemFragment_GridItemSize>())
			{
				if (ContainerFragment->FindEmptyPosition(ItemSizeFragment->ItemSize, Result))
				{
					Result.Direction = EGridInvSys_ItemDirection::Horizontal;
					return Result;
				}
				if (ContainerFragment->FindEmptyPosition(FIntPoint(ItemSizeFragment->ItemSize.Y, ItemSizeFragment->ItemSize.X), Result))
				{
					Result.Direction = EGridInvSys_ItemDirection::Vertical;
					return Result;
				}
			}
		}
	}
	return Result;
}

void UGridInvSys_InventoryComponent::UpdateItemInstancePosition(UInvSys_InventoryItemInstance* ItemInstance,
                                                                FGridInvSys_ItemPosition NewPosition)
{

	if (ItemInstance && ItemInstance->IsA<UGridInvSys_InventoryItemInstance>())
	{
		UGridInvSys_InventoryItemInstance* GridItem = Cast<UGridInvSys_InventoryItemInstance>(ItemInstance);
		check(GridItem)
		auto ContainerFragment = FindInventoryModule<UGridInvSys_InventoryModule_Container>(NewPosition.EquipSlotTag);
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
	const FGridInvSys_ItemPosition& NewPosition, bool bIsIgnoreInItemInstance) const
{
	if (NewPosition.IsValid() == false)
	{
		UE_LOG(LogInventorySystem, Warning, TEXT("%hs Falied, 传入的位置信息无效."), __FUNCTION__)
		return false;
	}

	auto ContainerFragment = FindInventoryModule<UGridInvSys_InventoryModule_Container>(NewPosition.EquipSlotTag);
	if (ContainerFragment)
	{
		return ContainerFragment->CheckItemPosition(ItemInstance, NewPosition, bIsIgnoreInItemInstance);
	}
	UE_LOG(LogInventorySystem, Warning, TEXT("%hs Falied, 在 %s 的库存组件中不存在片段 %s."),
		__FUNCTION__, *GetOwner()->GetName(), *NewPosition.EquipSlotTag.ToString())
	return false;
}

bool UGridInvSys_InventoryComponent::CancelOccupied(UInvSys_InventoryItemInstance* ItemInstance)
{
	if (ItemInstance == nullptr || ItemInstance->IsA<UGridInvSys_InventoryItemInstance>() == false)
	{
		UE_LOG(LogInventorySystem, Warning, TEXT("%hs Falied, ItemInstance is nullptr"), __FUNCTION__)
		return false;
	}
	UGridInvSys_InventoryItemInstance* GridItemInstance = Cast<UGridInvSys_InventoryItemInstance>(ItemInstance);
	FGameplayTag InventoryObjectTag = GridItemInstance->GetInventoryObjectTag();
	auto ContainerFragment = FindInventoryModule<UGridInvSys_InventoryModule_Container>(InventoryObjectTag);
	if (ContainerFragment)
	{
		ContainerFragment->UpdateContainerGridItemState(GridItemInstance, GridItemInstance->GetItemPosition(), false);
		return true;
	}
	UE_LOG(LogInventorySystem, Warning, TEXT("%hs Falied, 在 %s 的库存组件中不存在片段 %s."),
		__FUNCTION__, *GetOwner()->GetName(), *InventoryObjectTag.ToString())
	return false;
}

UGridInvSys_ContainerGridWidget* UGridInvSys_InventoryComponent::FindContainerGridWidget(FGameplayTag SlotTag,
                                                                                         int32 GridID)
{
	UInvSys_InventoryModule_Display* DisplayFragment =
		FindInventoryModule<UInvSys_InventoryModule_Display>(SlotTag);
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