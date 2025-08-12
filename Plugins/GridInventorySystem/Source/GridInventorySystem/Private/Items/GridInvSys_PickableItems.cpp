// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/GridInvSys_PickableItems.h"

#include "Components/GridInvSys_InventoryComponent.h"
#include "Components/InvSys_InventoryComponent.h"
#include "Components/InventoryObject/Fragment/InvSys_InventoryFragment_Container.h"
#include "Data/InvSys_ItemFragment_BaseItem.h"
#include "Data/InvSys_ItemFragment_PickUpItem.h"


// Sets default values
AGridInvSys_PickableItems::AGridInvSys_PickableItems()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

bool AGridInvSys_PickableItems::PickupItem(UInvSys_InventoryComponent* InvComp)
{
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

	UGridInvSys_InventoryComponent* GridInvComp = Cast<UGridInvSys_InventoryComponent>(InvComp);

	for (FGameplayTag ContainerTag : PickupItemFragment->ContainerPriority)
	{
		auto Container = GridInvComp->FindInventoryFragment<UInvSys_InventoryFragment_Container>(ContainerTag);
		if (Container == nullptr)
		{
			continue;
		}
		TArray<UInvSys_InventoryItemInstance*> StackableItems;
		Container->FindStackableItemInstances(ItemDefinition, StackableItems);
		for (int i = 0; ItemStackCount > 0; ++i)
		{
			if (StackableItems.IsValidIndex(i) == false)
			{
				break;
			}

			/*
			 * 循环遍历，直到 ItemStackCount == 0 或是 StackableItems 遍历完成
			 */

			int32 StackCount = StackableItems[i]->GetItemStackCount();
			int32 IncreaseStackCount = BaseItemFragment->MaxStackCount - StackCount;
			if (ItemStackCount < IncreaseStackCount)
			{
				IncreaseStackCount = ItemStackCount;
			}
			GridInvComp->UpdateItemStackCount(StackableItems[i], StackCount + IncreaseStackCount);
			// 更新结束条件: 当前值 - 增量
			ItemStackCount -= IncreaseStackCount;
		}
	}
	// 判断上一步堆叠物品是否将物品全部处理完成
	while (ItemStackCount > 0)
	{
		/*
		 * 循环遍历，直到 ItemStackCount == 0
		 */

		FGridInvSys_ItemPosition NewPosition;
		if (GridInvComp->FindEmptyPosition(ItemDefinition, NewPosition) == false)
		{
			UE_LOG(LogInventorySystem, Warning, TEXT("容器组件已满"))
			// todo::Broadcast Message?
			return false;
		}

		int32 NewStackCount = ItemStackCount > BaseItemFragment->MaxStackCount ? BaseItemFragment->MaxStackCount : ItemStackCount;
		GridInvComp->AddItemDefinitionToContainerPos(ItemDefinition, NewStackCount, NewPosition);

		// 更新结束条件: 当前值 - 增量
		ItemStackCount -= NewStackCount;
	}
	return true;
}

