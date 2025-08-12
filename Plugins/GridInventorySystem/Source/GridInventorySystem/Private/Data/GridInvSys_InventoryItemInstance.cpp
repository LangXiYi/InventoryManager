// Fill out your copyright notice in the Description page of Project Settings.


#include "Data/GridInvSys_InventoryItemInstance.h"

#include "GridInventorySystem.h"
#include "Components/InventoryObject/Fragment/GridInvSys_InventoryFragment_Container.h"
#include "Data/GridInvSys_ItemFragment_GridItemSize.h"
#include "Net/UnrealNetwork.h"

void UGridInvSys_InventoryItemInstance::PostReplicatedChange()
{
	Super::PostReplicatedChange();

	// UE_LOG(LogInventorySystem, Error, TEXT("Post Replicated Change %s"), bWaitPostRepNotify_ItemPosition ? TEXT("TRUE"):TEXT("FALSE"))
	if (bWaitPostRepNotify_ItemPosition)
	{
		bWaitPostRepNotify_ItemPosition = false;
		BroadcastItemPositionChangeMessage(LastItemPosition, ItemPosition);
	}
}

void UGridInvSys_InventoryItemInstance::OnTransferItems(UInvSys_InventoryFragment_Container* ContainerFragment)
{
	Super::OnTransferItems(ContainerFragment);
	ItemPosition.EquipSlotTag = ContainerFragment->GetInventoryTag();
}

void UGridInvSys_InventoryItemInstance::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UGridInvSys_InventoryItemInstance, ItemPosition);
}

void UGridInvSys_InventoryItemInstance::InitItemInstanceProps(const FGridInvSys_ItemPosition& NewItemPosition, bool bIsBroadcast)
{
	// 初始化物品实例时 LastItemPosition 与当前位置一致，直到下次更新
	SetItemPosition(NewItemPosition, bIsBroadcast);
}

void UGridInvSys_InventoryItemInstance::RemoveFromInventory()
{
	Super::RemoveFromInventory();

	// SetItemPosition(FGridInvSys_ItemPosition());
}

void UGridInvSys_InventoryItemInstance::SetItemPosition(const FGridInvSys_ItemPosition& NewItemPosition, bool bIsBroadcast)
{
	LastItemPosition = ItemPosition;
	ItemPosition = NewItemPosition;
	// UE_LOG(LogInventorySystem, Warning, TEXT("%s::%hs: %s %s ---> %s"),
	// 	*GPlayInEditorContextString, __FUNCTION__, *GetName(), *LastItemPosition.ToString(), *ItemPosition.ToString())

	if (bIsBroadcast)
	{
		BroadcastItemPositionChangeMessage(LastItemPosition, ItemPosition);
	}
	if (HasAuthority() && GetNetMode() != NM_DedicatedServer)
	{
		OnRep_ItemPosition(LastItemPosition);
	}
}

FIntPoint UGridInvSys_InventoryItemInstance::GetItemSize()
{
	return GetItemSize(ItemPosition.Direction);
}

FIntPoint UGridInvSys_InventoryItemInstance::GetItemSize(EGridInvSys_ItemDirection Direction)
{
	FIntPoint NativeItemSize = FIntPoint(0, 0);
	FIntPoint TargetItemSize = NativeItemSize;
	// 根据方向计算物体实际大小
	if (auto ItemSizeFragment = FindFragmentByClass<UGridInvSys_ItemFragment_GridItemSize>())
	{
		NativeItemSize = ItemSizeFragment->ItemSize;
	}
	else
	{
		checkNoEntry();
		return NativeItemSize;
	}
	// 计算旋转后的物品大小
	switch (Direction)
	{
	case EGridInvSys_ItemDirection::Horizontal:
		TargetItemSize = NativeItemSize;
		break;
	case EGridInvSys_ItemDirection::Vertical:
		TargetItemSize.X = NativeItemSize.Y;
		TargetItemSize.Y = NativeItemSize.X;
		break;
	default: ;
	}
	return TargetItemSize;
}

FIntPoint UGridInvSys_InventoryItemInstance::GetItemDefaultSize()
{
	return GetItemSize(EGridInvSys_ItemDirection::Horizontal);
}

void UGridInvSys_InventoryItemInstance::BroadcastItemPositionChangeMessage(const FGridInvSys_ItemPosition& OldPosition,
                                                                           const FGridInvSys_ItemPosition& NewPosition)
{
	FGridInvSys_ItemPositionChangeMessage ItemPositionChangeMessage;
	ItemPositionChangeMessage.ItemInstance = this;
	ItemPositionChangeMessage.InventoryComponent = GetInventoryComponent();
	ItemPositionChangeMessage.OldPosition = OldPosition;
	ItemPositionChangeMessage.NewPosition = NewPosition;

	UGameplayMessageSubsystem& GameplayMessageSubsystem = UGameplayMessageSubsystem::Get(GetWorld());
	GameplayMessageSubsystem.BroadcastMessage(Inventory_Message_ItemPositionChanged, ItemPositionChangeMessage);
}

void UGridInvSys_InventoryItemInstance::OnRep_ItemPosition(const FGridInvSys_ItemPosition& OldItemPosition)
{
	LastItemPosition = OldItemPosition;
	// UE_LOG(LogInventorySystem, Warning, TEXT("%s::%hs: %s %s ---> %s"),
	// 	*GPlayInEditorContextString, __FUNCTION__, *GetItemDisplayName().ToString(), *LastItemPosition.ToString(), *ItemPosition.ToString())
	// UE_LOG(LogInventorySystem, Error, TEXT("OnRep--->%s"), *GetName())
	if (ReplicateState == EInvSys_ReplicateState::PostChange)
	{
		bWaitPostRepNotify_ItemPosition = true;
	}
	// ON_REP_PROPERTY(ItemPosition)
}

void UGridInvSys_InventoryItemInstance::Execute_ItemPosition(const FGridInvSys_ItemPosition& OldItemPosition)
{
	// UE_LOG(LogInventorySystem, Warning, TEXT("%s:OnRep_ItemPosition:%s ---> %s"),
	// 	HasAuthority() ? TEXT("Server"):TEXT("Client"), *LastItemPosition.ToString(), *ItemPosition.ToString())
	/**
	 * 仅在当前位置与上次位置全部有效时广播通知
	 * 这样可以保证 Changed 事件与 Add、Remove 事件不会重复触发
	 */

	// if (LastItemPosition.IsValid() && ItemPosition.IsValid())
	{
		// BroadcastItemPositionChangeMessage(LastItemPosition, ItemPosition);
	}
}
