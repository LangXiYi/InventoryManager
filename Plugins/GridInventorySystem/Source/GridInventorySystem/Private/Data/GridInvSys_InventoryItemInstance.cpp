// Fill out your copyright notice in the Description page of Project Settings.


#include "Data/GridInvSys_InventoryItemInstance.h"

#include "GridInventorySystem.h"
#include "Net/UnrealNetwork.h"

void UGridInvSys_InventoryItemInstance::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UGridInvSys_InventoryItemInstance, ItemPosition);
}

void UGridInvSys_InventoryItemInstance::InitItemInstanceProps(const FGridInvSys_ItemPosition& NewItemPosition)
{
	// 初始化物品实例时 LastItemPosition 与当前位置一致，直到下次更新
	SetItemPosition(NewItemPosition);
}

void UGridInvSys_InventoryItemInstance::RemoveFromInventory()
{
	Super::RemoveFromInventory();

	SetItemPosition(FGridInvSys_ItemPosition());
}

void UGridInvSys_InventoryItemInstance::SetItemPosition(const FGridInvSys_ItemPosition& NewItemPosition)
{
	LastItemPosition = ItemPosition;
	ItemPosition = NewItemPosition;

	SetSlotTag(ItemPosition.EquipSlotTag);

	if (HasAuthority() && GetNetMode() != NM_DedicatedServer)
	{
		OnRep_ItemPosition(LastItemPosition);
	}
}

void UGridInvSys_InventoryItemInstance::BroadcastItemPositionChangeMessage(const FGridInvSys_ItemPosition& OldPosition,
	const FGridInvSys_ItemPosition& NewPosition)
{
	FGridInvSys_ItemPositionChangeMessage ItemPositionChangeMessage;
	ItemPositionChangeMessage.ItemInstance = this;
	ItemPositionChangeMessage.InvComp = GetInventoryComponent();
	ItemPositionChangeMessage.OldPosition = OldPosition;
	ItemPositionChangeMessage.NewPosition = NewPosition;

	UGameplayMessageSubsystem& GameplayMessageSubsystem = UGameplayMessageSubsystem::Get(GetWorld());
	GameplayMessageSubsystem.BroadcastMessage(Inventory_Message_ItemPositionChanged, ItemPositionChangeMessage);
}

void UGridInvSys_InventoryItemInstance::OnRep_ItemPosition(const FGridInvSys_ItemPosition& OldItemPosition)
{
	ON_REP_PROPERTY(ItemPosition)
}

void UGridInvSys_InventoryItemInstance::Execute_ItemPosition(const FGridInvSys_ItemPosition& OldItemPosition)
{
	LastItemPosition = OldItemPosition;
	BroadcastItemPositionChangeMessage(OldItemPosition, ItemPosition);
}
