// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/GridInvSys_PickableItems.h"

#include "Components/GridInvSys_InventoryComponent.h"
#include "Components/InvSys_InventoryComponent.h"
#include "Components/InventoryObject/Fragment/InvSys_InventoryModule_Container.h"
#include "Data/InvSys_ItemFragment_BaseItem.h"
#include "Data/InvSys_ItemFragment_EquipItem.h"
#include "Data/InvSys_ItemFragment_PickUpItem.h"


// Sets default values
AGridInvSys_PickableItems::AGridInvSys_PickableItems()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

bool AGridInvSys_PickableItems::PickupItem(UInvSys_InventoryComponent* InvComp, bool bIsAutoEquip)
{
	if (PickableItemInstance == nullptr)
	{
		UE_LOG(LogInventorySystem, Error, TEXT("%hs Falied, PickableItemInstance is nullptr."), __FUNCTION__)
		return false;
	}
	int32 ItemStackCount = GetItemStackCount();
	if (ItemStackCount <= 0)
	{
		UE_LOG(LogInventorySystem, Error, TEXT("%hs Falied, ItemStackCount Less zero."), __FUNCTION__)
		return false;
	}
	if (InvComp == nullptr || InvComp->IsA<UGridInvSys_InventoryComponent>() == false)
	{
		UE_LOG(LogInventorySystem, Error, TEXT("%hs Falied, InventoryComponent is nullptr."), __FUNCTION__)
		return false;
	}
	auto PickupItemFragment = FindFragmentByClass<UInvSys_ItemFragment_PickUpItem>();
	if (PickupItemFragment == nullptr)
	{
		UE_LOG(LogInventorySystem, Error, TEXT("%hs Falied, PickupItemFragment is nullptr."), __FUNCTION__)
		return false;
	}
	auto BaseItemFragment = FindFragmentByClass<UInvSys_ItemFragment_BaseItem>();
	if (BaseItemFragment == nullptr)
	{
		UE_LOG(LogInventorySystem, Error, TEXT("%hs Falied, StackableFragment is nullptr."), __FUNCTION__)
		return false;
	}
	int32 MaxStackCount = BaseItemFragment->MaxStackCount;
	UGridInvSys_InventoryComponent* GridInvComp = Cast<UGridInvSys_InventoryComponent>(InvComp);

	/**
	 * 自动装备物品，若物品可堆叠则根据有效最大值装备物品
	 */
	if (bIsAutoEquip)
	{
		auto EquipItemFragment = FindFragmentByClass<UInvSys_ItemFragment_EquipItem>();
		if (EquipItemFragment)
		{
			int32 SupportSlotCount = EquipItemFragment->SupportEquipSlot.Num();
			bool bIsSuccessEquip = false;
			for (int i = 0; i < SupportSlotCount; ++i)
			{
				FGameplayTag InventoryTag = EquipItemFragment->SupportEquipSlot.GetByIndex(i);
				if (GridInvComp->IsValidInventoryTag(InventoryTag) == false)
				{
					continue;
				}
				if (GridInvComp->HasEquippedItemInstance(InventoryTag))
				{
					continue;
				}
				// if (ItemStackCount <= MaxStackCount)
				{
					// 将物品之间装备至目标位置
					GridInvComp->EquipItemInstance(PickableItemInstance, InventoryTag);
					return true;
				}
				// 仍然剩余部分物品，此时会继续循环查找可装备的槽位，若循环结束，则将剩余物品添加至容器内。
				// GridInvComp->EquipItemDefinition(PickableItemInstance->GetItemDefinition(), InventoryTag, MaxStackCount);
				// ItemStackCount -= MaxStackCount;
				// bIsSuccessEquip = true;
			}
			if (bIsSuccessEquip == false && PickableItemInstance->PayloadItems.IsEmpty() == false)
			{
				// 目标物品内部存在其他物品且不能直接装备
				// todo::尝试将该物品以及内部物品一起添加到背包中
				return false;
			}
		}
	}

	// 根据优先级遍历容器，优先填充未达到最大堆叠数量的物品
	for (FGameplayTag ContainerTag : PickupItemFragment->ContainerPriority)
	{
		auto Container = GridInvComp->FindInventoryModule<UInvSys_InventoryModule_Container>(ContainerTag);
		if (Container == nullptr)
		{
			continue;
		}
		// 查找所有未达到最大堆叠数量的同类物品
		TArray<UInvSys_InventoryItemInstance*> StackableItems;
		Container->FindStackableItemInstances(PickableItemInstance, StackableItems);

		for (UInvSys_InventoryItemInstance* StackableItem : StackableItems)
		{
			if (ItemStackCount <= 0)
			{
				break;
			}
			StackableItem->SuperposeItemInstance(PickableItemInstance);

			ItemStackCount = PickableItemInstance->GetItemStackCount();
			if (ItemStackCount <= 0)
			{
				PickableItemInstance->RemoveAndDestroyFromInventory();
				PickableItemInstance->ConditionalBeginDestroy();
			}
		}
		/*for (int i = 0; ItemStackCount > 0; ++i)
		{
			if (StackableItems.IsValidIndex(i) == false)
			{
				break;
			}

			/*
			 * 循环遍历，直到 ItemStackCount == 0 或是 StackableItems 遍历完成
			 #1#

			int32 StackCount = StackableItems[i]->GetItemStackCount();
			int32 IncreaseStackCount = MaxStackCount - StackCount;
			if (ItemStackCount < IncreaseStackCount)
			{
				IncreaseStackCount = ItemStackCount;
			}
			GridInvComp->UpdateItemStackCount(StackableItems[i], StackCount + IncreaseStackCount);
			// 更新结束条件: 当前值 - 增量
			ItemStackCount = IncreaseStackCount;
		}*/
	}
	// 库存内所有同类物品的堆叠数量已经达到最大值，若此时仍有剩余数量，则会添加新的物品实例到容器内
	// while (ItemStackCount > 0)
	if (ItemStackCount > 0)
	{
		/*
		 * 循环遍历，直到 ItemStackCount == 0
		 */

		FGridInvSys_ItemPosition NewPosition;
		if (GridInvComp->FindEmptyPosition(PickableItemInstance, NewPosition) == false)
		{
			// todo::是否需要广播该事件？
			// UE_LOG(LogInventorySystem, Warning, TEXT("容器组件已满"))
			SetItemStackCount(ItemStackCount); // 所有修改操作完成后，更新物品实例的堆叠数量
			return false;
		}

		// UE_LOG(LogInventorySystem, Warning, TEXT("找到有效位置 %s"), *NewPosition.ToString())
		int32 NewStackCount = ItemStackCount > MaxStackCount ? MaxStackCount : ItemStackCount;
		// todo::除了位置你是否还有其他需要添加的属性？
		// GridInvComp->AddItemDefinitionToContainerPos(PickableItemInstance->GetItemDefinition(), NewStackCount, NewPosition);
		// UInvSys_InventoryItemInstance* NewItemInstance = GridInvComp->AddItemDefinitionToPos(
		// 	PickableItemInstance->GetClass(),
		// 	PickableItemInstance->GetItemDefinition(),
		// 	NewStackCount, NewPosition);
		GridInvComp->AddItemInstanceToContainerPos(PickableItemInstance, NewPosition);
		// 更新结束条件: 当前值 - 增量
		ItemStackCount -= NewStackCount;
	}
	// SetItemStackCount(ItemStackCount); // 所有修改操作完成后，更新物品实例的堆叠数量
	// if (ItemStackCount <= 0)
	// {
	// 	PickableItemInstance->ConditionalBeginDestroy();
	// }
	return true;
}

