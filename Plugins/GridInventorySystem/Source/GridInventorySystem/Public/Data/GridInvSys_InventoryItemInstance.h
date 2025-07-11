// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseInventorySystem.h"
#include "GridInvSys_CommonType.h"
#include "Components/InvSys_InventoryComponent.h"
#include "Data/InvSys_InventoryItemInstance.h"
#include "GridInvSys_InventoryItemInstance.generated.h"

USTRUCT()
struct FGridInvSys_ItemPositionChangeMessage{
	GENERATED_BODY()

	UPROPERTY()
	UInvSys_InventoryItemInstance* Instance;

	UPROPERTY()
	FGridInvSys_ItemPosition OldPosition;

	UPROPERTY()
	FGridInvSys_ItemPosition NewPosition;
};

DECLARE_DELEGATE_OneParam(FOnItemPositionChange, const FGridInvSys_ItemPositionChangeMessage&);

/**
 * 
 */
UCLASS()
class GRIDINVENTORYSYSTEM_API UGridInvSys_InventoryItemInstance : public UInvSys_InventoryItemInstance
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void InitItemInstanceProps(const FGridInvSys_ItemPosition& NewItemPosition)
	{
		SetItemPosition(NewItemPosition);
	}

	virtual void RemoveFromInventory() override;
	
	void SetItemPosition(const FGridInvSys_ItemPosition& NewItemPosition)
	{
		ItemPosition = NewItemPosition;

		SetSlotTag(ItemPosition.EquipSlotTag);

		UWorld* World = GetWorld();
		if (World && World->GetNetMode() != NM_DedicatedServer)
		{
			OnRep_ItemPosition();
		}
	}

	FGridInvSys_ItemPosition GetItemPosition() const
	{
		return ItemPosition;
	}

	void BroadcastItemPositionChangeMessage(const FGridInvSys_ItemPosition& OldPosition, const FGridInvSys_ItemPosition& NewPosition)
	{
		FGridInvSys_ItemPositionChangeMessage ItemPositionChangeMessage;
		ItemPositionChangeMessage.Instance = this;
		ItemPositionChangeMessage.OldPosition = OldPosition;
		ItemPositionChangeMessage.NewPosition = NewPosition;

		if (OnItemPositionChange.ExecuteIfBound(ItemPositionChangeMessage))
		{
			UE_LOG(LogInventorySystem, Log, TEXT("[%s]广播物品位置变化事件 ===> %s:%d[%d,%d]"),
				InvComp->HasAuthority() ? TEXT("Server") : TEXT("Client"),
				*ItemPosition.EquipSlotTag.ToString(), NewPosition.GridID, NewPosition.Position.X, NewPosition.Position.Y);
		}
	}
	
	FOnItemPositionChange& OnItemPositionChangeDelegate()
	{
		return OnItemPositionChange;
	}
	
protected:
	UPROPERTY(ReplicatedUsing = OnRep_ItemPosition)
	FGridInvSys_ItemPosition ItemPosition;

	FGridInvSys_ItemPosition LastItemPosition;

	UFUNCTION()
	void OnRep_ItemPosition()
	{
		UE_LOG(LogInventorySystem, Warning, TEXT("=== OnRep_ItemPosition [%s:Begin] ==="),
			InvComp->HasAuthority() ? TEXT("Server"):TEXT("Client"))
		BroadcastItemPositionChangeMessage(LastItemPosition, ItemPosition);
		LastItemPosition = ItemPosition;
	}

private:
	FOnItemPositionChange OnItemPositionChange;
};
