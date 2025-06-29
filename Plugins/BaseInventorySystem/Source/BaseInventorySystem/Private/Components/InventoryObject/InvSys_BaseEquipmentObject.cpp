// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/InventoryObject/InvSys_BaseEquipmentObject.h"

#include "Net/UnrealNetwork.h"

UInvSys_BaseEquipmentObject::UInvSys_BaseEquipmentObject()
{
}

void UInvSys_BaseEquipmentObject::AddInventoryItemToEquipSlot(const FInvSys_InventoryItem& NewItem,
                                                              FName TargetSlotName)
{
	if (HasAuthority())
	{
		UE_LOG(LogInventorySystem, Log, TEXT("[%s:%s] 添加装备 [%s] ===> [%s]"),
			HasAuthority() ? TEXT("Server") : TEXT("Client"),
			*GetInventoryComponent()->GetOwner()->GetName(),
			*NewItem.ItemID.ToString(), *TargetSlotName.ToString());
		Occupant = NewItem;
		// 手动调用OnRep函数更新服务器玩家的数据
		if (GetNetMode() != NM_DedicatedServer && IsLocalController())
		{
			OnRep_Occupant(Occupant);
		}
	}
}

void UInvSys_BaseEquipmentObject::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UInvSys_BaseEquipmentObject, Occupant, COND_None);
}

void UInvSys_BaseEquipmentObject::OnRep_Occupant(FInvSys_InventoryItem OldOccupant)
{
	TryRefreshOccupant();
}