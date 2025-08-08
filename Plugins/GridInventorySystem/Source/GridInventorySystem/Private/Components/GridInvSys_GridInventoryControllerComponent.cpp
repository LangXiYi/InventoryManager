// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/GridInvSys_GridInventoryControllerComponent.h"

#include "Components/GridInvSys_InventoryComponent.h"
#include "Components/InventoryObject/Fragment/GridInvSys_InventoryFragment_Container.h"
#include "Data/GridInvSys_InventoryItemInstance.h"
#include "Library/GridInvSys_CommonFunctionLibrary.h"


UGridInvSys_GridInventoryControllerComponent::UGridInvSys_GridInventoryControllerComponent()
{
}

void UGridInvSys_GridInventoryControllerComponent::TryDropItemInstance(UInvSys_InventoryComponent* InvComp,
	UInvSys_InventoryItemInstance* InItemInstance, const FGridInvSys_ItemPosition& InPos)
{
	bool bIsSuccess = false;
	UGridInvSys_InventoryItemInstance* GridItemInstance = Cast<UGridInvSys_InventoryItemInstance>(InItemInstance);
	const FGridInvSys_ItemPosition& OldPosition = GridItemInstance->GetItemPosition();
	if (GridItemInstance)
	{
		bIsSuccess = DropItemInstance<UGridInvSys_InventoryItemInstance>(InvComp, GridItemInstance,
			InPos.EquipSlotTag, InPos);
	}
	UE_CLOG(PRINT_INVENTORY_SYSTEM_LOG, LogInventorySystem, Log, TEXT("[Server:%s] 更新物品位置信息[%s] {%s}--> {%s}"),
		bIsSuccess ? TEXT("TRUE") : TEXT("FALSE"),
		*InItemInstance->GetItemDisplayName().ToString(),
		*OldPosition.ToString(), *InPos.ToString())
}

void UGridInvSys_GridInventoryControllerComponent::Server_CancelOccupied_Implementation(
	UInvSys_InventoryItemInstance* ItemInstance)
{
	check(ItemInstance)
	if (ItemInstance && ItemInstance->IsA<UGridInvSys_InventoryItemInstance>())
	{
		UGridInvSys_InventoryComponent* InvComp = ItemInstance->GetInventoryComponent<UGridInvSys_InventoryComponent>();
		check(InvComp)
		if (InvComp)
		{
			InvComp->CancelOccupied((UGridInvSys_InventoryItemInstance*)ItemInstance);
		}
	}
}

void UGridInvSys_GridInventoryControllerComponent::Server_RemoveItemInstance_Implementation(UInvSys_InventoryItemInstance* ItemInstance)
{
	check(ItemInstance)
	if (ItemInstance)
	{
		UInvSys_InventoryComponent* InvComp = ItemInstance->GetInventoryComponent();
		check(InvComp)
		if (InvComp)
		{
			InvComp->RemoveItemInstance(ItemInstance);
		}
	}
}

void UGridInvSys_GridInventoryControllerComponent::Server_SwapItemInstances_Implementation(
	UInvSys_InventoryComponent* ToInvComp, UGridInvSys_InventoryItemInstance* ItemInstance, const FGridInvSys_ItemPosition& ToPosition,
	const TArray<UGridInvSys_InventoryItemInstance*>& TargetItemInstances,
	const TArray<FGridInvSys_ItemPosition>& TargetPositions)
{
	check(TargetItemInstances.Num() == TargetPositions.Num())
	bool bIsSuccess = true;
	UGridInvSys_InventoryComponent* LastInvComp = ItemInstance->GetInventoryComponent<UGridInvSys_InventoryComponent>();
	check(LastInvComp)
	if (LastInvComp)
	{
		UE_LOG(LogInventorySystem, Warning, TEXT("Server Remove Item %s"), *ItemInstance->GetItemPosition().ToString())
		if (LastInvComp->RemoveItemInstance(ItemInstance))
		{
			for (int i = 0; i < TargetItemInstances.Num(); ++i)
			{
				if (TargetPositions.IsValidIndex(i) == false)
				{
					continue;
				}
				UInvSys_InventoryComponent* TempInvComp = TargetItemInstances[i]->GetInventoryComponent();
				if (LastInvComp->CheckItemPosition(TargetItemInstances[i], TargetPositions[i]))
				{
					// todo::是否需要添加额外的权限验证？
					bool bIsRemoved = TempInvComp->RemoveItemInstance(TargetItemInstances[i]);
					if (bIsRemoved)
					{
						LastInvComp->AddItemInstance<UGridInvSys_InventoryItemInstance>(TargetItemInstances[i],
							TargetPositions[i].EquipSlotTag, TargetPositions[i]);
					}
					else
					{
						UE_LOG(LogInventorySystem, Error, TEXT("%hs Falied, 无法移除 %s。"), __FUNCTION__, *TargetItemInstances[i]->GetItemDisplayName().ToString())
					}
				}
			}
		}
		else
		{
			bIsSuccess = false;
			UE_LOG(LogInventorySystem, Error, TEXT("%hs Falied, 无法移除 %s。"), __FUNCTION__, *ItemInstance->GetItemDisplayName().ToString())
		}
	}

	check(ToInvComp)
	if (ToInvComp && bIsSuccess)
	{
		ToInvComp->AddItemInstance<UGridInvSys_InventoryItemInstance>(ItemInstance, ToPosition.EquipSlotTag, ToPosition);	
	}
}

void UGridInvSys_GridInventoryControllerComponent::Server_AddItemInstancesToContainerPos_Implementation(
	UInvSys_InventoryComponent* InvComp, const TArray<UInvSys_InventoryItemInstance*>& InItemInstances,
	const TArray<FGridInvSys_ItemPosition>& InPosArray)
{
	check(InvComp)
	if (InvComp && InvComp->IsA<UGridInvSys_InventoryComponent>())
	{
		UGridInvSys_InventoryComponent* GridInvComp = Cast<UGridInvSys_InventoryComponent>(InvComp);
		int32 ItemNum = InItemInstances.Num();
		int32 PosNum = InPosArray.Num();

		check(GridInvComp)
		check(ItemNum == PosNum)
		if (GridInvComp && ItemNum == PosNum)
		{
			for (int i = 0; i < ItemNum; ++i)
			{
				GridInvComp->AddItemInstanceToContainerPos(InItemInstances[i], InPosArray[i]);
			}
		}
	}
}

void UGridInvSys_GridInventoryControllerComponent::Server_RestoreItemInstanceToPos_Implementation(
	UInvSys_InventoryComponent* InvComp, UInvSys_InventoryItemInstance* InItemInstance,
	const FGridInvSys_ItemPosition& InPos)
{
	check(InvComp)
	if (InvComp && InvComp->IsA<UGridInvSys_InventoryComponent>())
	{
		UGridInvSys_InventoryComponent* GridInvComp = Cast<UGridInvSys_InventoryComponent>(InvComp);
		GridInvComp->RestoreItemInstanceToPos(InItemInstance, InPos);
	}
}

void UGridInvSys_GridInventoryControllerComponent::Server_UpdateItemInstancePosition_Implementation(
	UInvSys_InventoryComponent* InvComp, UInvSys_InventoryItemInstance* ItemInstance,
	FGridInvSys_ItemPosition NewPosition)
{
	check(ItemInstance)
	if (ItemInstance)
	{
		// UGridInvSys_InventoryItemInstance* GridItem = Cast<UGridInvSys_InventoryItemInstance>(ItemInstance);
		UGridInvSys_InventoryComponent* GridComp = ItemInstance->GetInventoryComponent<UGridInvSys_InventoryComponent>();
		check(GridComp)
		if (GridComp)
		{
			GridComp->UpdateItemInstancePosition(ItemInstance, NewPosition);
			// OldInvComp->RemoveItemInstance(ItemInstance); // 将物品从原先的库存组件中删除
			// InvComp->AddItemInstance<UGridInvSys_InventoryItemInstance>(GridItem, NewPosition.EquipSlotTag, NewPosition);
		}
	}
}

void UGridInvSys_GridInventoryControllerComponent::Server_SwapItemInstance_Implementation(
	UGridInvSys_InventoryItemInstance* FromItemInstance, UGridInvSys_InventoryItemInstance* ToItemInstance)
{
	check(FromItemInstance)
	check(ToItemInstance)
	// 两物品实例大小相同
	if (FromItemInstance != ToItemInstance && FromItemInstance->GetItemSize() == ToItemInstance->GetItemSize())
	{
		UInvSys_InventoryComponent* FromInvComp = FromItemInstance->GetInventoryComponent();
		UInvSys_InventoryComponent* ToInvComp = ToItemInstance->GetInventoryComponent();
		if (FromInvComp && ToInvComp)
		{
			// 这里需要提前缓存ToItem的位置信息，避免在第一次添加执行完成后修改FromItem到To位置时的数据被污染。
			FGameplayTag FromItemTag = FromItemInstance->GetSlotTag();
			FGridInvSys_ItemPosition FromItemPosition = FromItemInstance->GetItemPosition();
			
			FGameplayTag ToItemTag = ToItemInstance->GetSlotTag();
			FGridInvSys_ItemPosition ToItemPosition = ToItemInstance->GetItemPosition();

			FromInvComp->RemoveItemInstance(FromItemInstance);
			ToInvComp->RemoveItemInstance(ToItemInstance);

			FromInvComp->AddItemInstance<UGridInvSys_InventoryItemInstance>(ToItemInstance, FromItemTag, FromItemPosition);
			ToInvComp->AddItemInstance<UGridInvSys_InventoryItemInstance>(FromItemInstance, ToItemTag, ToItemPosition);
		}
	}
}

void UGridInvSys_GridInventoryControllerComponent::Server_TryDropItemInstance_Implementation(
	UInvSys_InventoryComponent* InvComp, UInvSys_InventoryItemInstance* InItemInstance,
	const FGridInvSys_ItemPosition& InPos)
{
	if (InvComp == nullptr || InItemInstance == nullptr)
	{
		checkf(false, TEXT("传入的库存组件或物品实例为空"));
		return;
	}
	bool bIsSuccess = false;
	auto ContainerFragment = InvComp->FindInventoryObjectFragment<UGridInvSys_InventoryFragment_Container>(InPos.EquipSlotTag);
	if (InItemInstance)
	{
		FIntPoint ItemSize = UGridInvSys_CommonFunctionLibrary::CalculateItemInstanceSizeFrom(InItemInstance, InPos.Direction);
		bool bHasEnoughFreeSpace = ContainerFragment->HasEnoughFreeSpace(InPos.Position, InPos.GridID, ItemSize);

		UE_CLOG(bHasEnoughFreeSpace == false, LogInventorySystem, Error, TEXT("位置 %s 无法容纳大小为 %s 的 %s"),
			*InPos.ToString(), *ItemSize.ToString(), *InItemInstance->GetItemDisplayName().ToString())

		if (bHasEnoughFreeSpace)
		{
			bIsSuccess = DropItemInstance<UGridInvSys_InventoryItemInstance>(InvComp, InItemInstance, InPos.EquipSlotTag, InPos);
		}
	}
	UE_CLOG(bIsSuccess == false, LogInventorySystem, Error, TEXT("放置 %s 失败 ---> %s"),
		*InItemInstance->GetItemDisplayName().ToString(), *InPos.ToString())
}
