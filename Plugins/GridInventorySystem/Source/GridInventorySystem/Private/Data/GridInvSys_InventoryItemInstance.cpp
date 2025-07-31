// Fill out your copyright notice in the Description page of Project Settings.


#include "Data/GridInvSys_InventoryItemInstance.h"

#include "GridInventorySystem.h"
#include "Net/UnrealNetwork.h"

void UGridInvSys_InventoryItemInstance::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UGridInvSys_InventoryItemInstance, ItemPosition);
	DOREPLIFETIME(UGridInvSys_InventoryItemInstance, LastItemPosition);
}

void UGridInvSys_InventoryItemInstance::InitItemInstanceProps(const FGridInvSys_ItemPosition& NewItemPosition)
{
	// 初始化物品实例时 LastItemPosition 与当前位置一致，直到下次更新
	LastItemPosition = NewItemPosition;
	ItemPosition = NewItemPosition;
	SetSlotTag(ItemPosition.EquipSlotTag);
}

void UGridInvSys_InventoryItemInstance::RemoveFromInventory()
{
	Super::RemoveFromInventory();

	//SetItemPosition(FGridInvSys_ItemPosition());
}

void UGridInvSys_InventoryItemInstance::SetItemPosition(const FGridInvSys_ItemPosition& NewItemPosition)
{
	LastItemPosition = ItemPosition;
	ItemPosition = NewItemPosition;
	SetSlotTag(ItemPosition.EquipSlotTag);

	if (Owner->HasAuthority() && Owner->GetNetMode() != NM_DedicatedServer)
	{
		OnRep_ItemPosition();
	}
}

void UGridInvSys_InventoryItemInstance::BroadcastItemPositionChangeMessage(const FGridInvSys_ItemPosition& OldPosition,
	const FGridInvSys_ItemPosition& NewPosition)
{
	FGridInvSys_ItemPositionChangeMessage ItemPositionChangeMessage;
	ItemPositionChangeMessage.InvComp = GetInventoryComponent();
	ItemPositionChangeMessage.InventoryObjectTag = GetSlotTag();
	ItemPositionChangeMessage.ItemInstance = this;
	ItemPositionChangeMessage.OldPosition = OldPosition;
	ItemPositionChangeMessage.NewPosition = NewPosition;

	UE_CLOG(PRINT_INVENTORY_SYSTEM_LOG, LogInventorySystem, Warning, TEXT("%s:Broadcast Item Position Changed"), Owner->HasAuthority() ? TEXT("Server"):TEXT("Client"))

	UGameplayMessageSubsystem& GameplayMessageSubsystem = UGameplayMessageSubsystem::Get(GetWorld());
	GameplayMessageSubsystem.BroadcastMessage(Inventory_Message_ItemPositionChanged, ItemPositionChangeMessage);
}

void UGridInvSys_InventoryItemInstance::OnRep_ItemPosition()
{
	if (LastItemPosition != ItemPosition)
	{
		BroadcastItemPositionChangeMessage(LastItemPosition, ItemPosition);
	}
}
