// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/GridInvSys_GridInventoryControllerComponent.h"

#include "Components/GridInvSys_InventoryComponent.h"
#include "Components/InventoryObject/Fragment/GridInvSys_InventoryModule_Container.h"
#include "Data/GridInvSys_InventoryItemInstance.h"
#include "Data/InvSys_ItemFragment_BaseItem.h"
#include "Library/GridInvSys_CommonFunctionLibrary.h"


UGridInvSys_GridInventoryControllerComponent::UGridInvSys_GridInventoryControllerComponent()
{
}

void UGridInvSys_GridInventoryControllerComponent::Server_SplitItemInstance_Implementation(
	UInvSys_InventoryItemInstance* ItemInstance, int32 SplitSize)
{
	if (ItemInstance == nullptr)
	{
		UE_LOG(LogInventorySystem, Error, TEXT("%hs Falied, ItemInstance is nullptr."), __FUNCTION__)
		return;
	}
	if (SplitSize >= ItemInstance->GetItemStackCount())
	{
		UE_LOG(LogInventorySystem, Error, TEXT("%hs Falied, SplitSize >= ItemStackCount."), __FUNCTION__)
		return;
	}
	UGridInvSys_InventoryComponent* GridInvComp = ItemInstance->GetInventoryComponent<UGridInvSys_InventoryComponent>();
	if (GridInvComp == nullptr)
	{
		UE_LOG(LogInventorySystem, Error, TEXT("%hs Falied, GridInvComp is nullptr."), __FUNCTION__)
		return;
	}
	FGridInvSys_ItemPosition OutPosition;
	if (GridInvComp->FindEmptyPosition(ItemInstance, OutPosition))
	{
		GridInvComp->SplitItemInstance<UGridInvSys_InventoryItemInstance>(
			ItemInstance, SplitSize, OutPosition.EquipSlotTag, OutPosition);
	}
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
	if (ItemInstance == nullptr)
	{
		UE_LOG(LogInventorySystem, Error, TEXT("%hs Falied, ItemInstance is nullptr."), __FUNCTION__)
		return;
	}

	if (ItemInstance->PayloadItems.IsEmpty() == false)
	{
		UE_LOG(LogInventorySystem, Error, TEXT("%hs Falied, ItemInstance->PayloadItems.Num = %d."), __FUNCTION__, ItemInstance->PayloadItems.Num())
		return;
	}

	if (TargetItemInstances.Num() != TargetPositions.Num())
	{
		checkf(false, TEXT("物品实例与位置信息数量不匹配，存在无效索引"))
		return;
	}
	bool bIsSuccess = true;
	UGridInvSys_InventoryComponent* LastInvComp = ItemInstance->GetInventoryComponent<UGridInvSys_InventoryComponent>();
	check(LastInvComp)
	if (LastInvComp)
	{
		if (LastInvComp->RemoveItemInstance(ItemInstance))
		{
			for (int i = 0; i < TargetItemInstances.Num(); ++i)
			{
				UInvSys_InventoryComponent* TempInvComp = TargetItemInstances[i]->GetInventoryComponent();
				// UE_LOG(LogInventorySystem, Log, TEXT("目标物品 %s 的所有者为--%s") ,
				// 	*TargetItemInstances[i]->GetItemDisplayName().ToString() ,*TempInvComp->GetOwner()->GetName());
				if (LastInvComp->CheckItemPosition(TargetItemInstances[i], TargetPositions[i]))
				{
					// todo::是否需要添加额外的权限验证？
					bool bIsRemoved = TempInvComp->RemoveItemInstance(TargetItemInstances[i]);
					if (bIsRemoved)
					{
						LastInvComp->AddItemInstance<UGridInvSys_InventoryItemInstance>(TargetItemInstances[i],
							TargetPositions[i].EquipSlotTag, TargetPositions[i]);
					}
				}
			}
		}
	}

	check(ToInvComp)
	if (ToInvComp && bIsSuccess)
	{
		ToInvComp->AddItemInstance<UGridInvSys_InventoryItemInstance>(ItemInstance, ToPosition.EquipSlotTag, ToPosition);	
	}
}

void UGridInvSys_GridInventoryControllerComponent::Server_AddItemInstancesToContainerPos_Implementation(
	UInvSys_InventoryComponent* InvComp, UInvSys_InventoryItemInstance* InItemInstance, FGridInvSys_ItemPosition InPos)
{
	check(InvComp)
	if (InvComp && InvComp->IsA<UGridInvSys_InventoryComponent>())
	{
		UGridInvSys_InventoryComponent* GridInvComp = Cast<UGridInvSys_InventoryComponent>(InvComp);
		GridInvComp->AddItemInstanceToContainerPos(InItemInstance, InPos);
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
	if (FromItemInstance == nullptr || ToItemInstance == nullptr)
	{
		UE_LOG(LogInventorySystem, Error, TEXT("%hs Falied, ItemInstance is nullptr."), __FUNCTION__)
		return;
	}

	if (FromItemInstance == ToItemInstance)
	{
		UE_LOG(LogInventorySystem, Error, TEXT("%hs Falied, FromItemInstance == ToItemInstance."), __FUNCTION__)
		return;
	}

	if (FromItemInstance->PayloadItems.IsEmpty() == false)
	{
		UE_LOG(LogInventorySystem, Error, TEXT("%hs Falied, FromItemInstance->PayloadItems.Num = %d."), __FUNCTION__, FromItemInstance->PayloadItems.Num())
		return;
	}

	if (ToItemInstance->PayloadItems.IsEmpty() == false)
	{
		UE_LOG(LogInventorySystem, Error, TEXT("%hs Falied, ToItemInstance->PayloadItems.Num = %d."), __FUNCTION__, FromItemInstance->PayloadItems.Num())
		return;
	}

	// 互换它们的位置
	UInvSys_InventoryComponent* FromInvComp = FromItemInstance->GetInventoryComponent();
	UInvSys_InventoryComponent* ToInvComp = ToItemInstance->GetInventoryComponent();
	if (FromInvComp == nullptr || ToInvComp == nullptr)
	{
		UE_LOG(LogInventorySystem, Error, TEXT("%hs Falied, InventoryComponent is nullptr."), __FUNCTION__)
		return;
	}

	// 两物品实例大小相同
	if (FromItemInstance->GetItemSize() == ToItemInstance->GetItemSize())
	{
		// 这里需要提前缓存ToItem的位置信息，避免在第一次添加执行完成后修改FromItem到To位置时的数据被污染。
		FGameplayTag FromItemTag = FromItemInstance->GetInventoryObjectTag();
		FGridInvSys_ItemPosition FromItemPosition = FromItemInstance->GetItemPosition();
		
		FGameplayTag ToItemTag = ToItemInstance->GetInventoryObjectTag();
		FGridInvSys_ItemPosition ToItemPosition = ToItemInstance->GetItemPosition();

		FromInvComp->RemoveItemInstance(FromItemInstance);
		ToInvComp->RemoveItemInstance(ToItemInstance);

		FromInvComp->AddItemInstance<UGridInvSys_InventoryItemInstance>(ToItemInstance, FromItemTag, FromItemPosition);
		ToInvComp->AddItemInstance<UGridInvSys_InventoryItemInstance>(FromItemInstance, ToItemTag, ToItemPosition);
	}
}

void UGridInvSys_GridInventoryControllerComponent::Server_TryDropItemInstance_Implementation(
	UInvSys_InventoryComponent* InvComp, UInvSys_InventoryItemInstance* InItemInstance,
	const FGridInvSys_ItemPosition& InPos)
{
	UGridInvSys_InventoryComponent* GridInvComp = Cast<UGridInvSys_InventoryComponent>(InvComp);
	if (InvComp && InItemInstance)
	{
		if (InItemInstance->PayloadItems.IsEmpty())
		{
			auto ContainerModlue = GridInvComp->FindInventoryModule<UGridInvSys_InventoryModule_Container>(InPos.EquipSlotTag);
			FIntPoint ItemSize = UGridInvSys_CommonFunctionLibrary::CalculateItemInstanceSizeFrom(InItemInstance, InPos.Direction);
			if (ContainerModlue && ContainerModlue->IsUnoccupiedInSquareRange(InPos.GridID, InPos.Position, ItemSize))
			{
				DropItemInstance<UGridInvSys_InventoryItemInstance>(InvComp, InItemInstance, InPos.EquipSlotTag, InPos);
			}
		}
	}
	CancelDragItemInstance(InItemInstance);
}
