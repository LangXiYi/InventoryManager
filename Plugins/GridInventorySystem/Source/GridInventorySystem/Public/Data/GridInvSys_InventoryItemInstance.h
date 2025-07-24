// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseInventorySystem.h"
#include "GridInvSys_CommonType.h"
#include "Components/InvSys_InventoryComponent.h"
#include "Data/InvSys_InventoryItemInstance.h"
#include "GridInvSys_InventoryItemInstance.generated.h"

class UGridInvSys_ContainerGridWidget;

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
		LastItemPosition = ItemPosition;
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

	FGridInvSys_ItemPosition GetLastItemPosition() const
	{
		return LastItemPosition;
	}

	void BroadcastItemPositionChangeMessage(const FGridInvSys_ItemPosition& OldPosition, const FGridInvSys_ItemPosition& NewPosition)
	{
		FGridInvSys_ItemPositionChangeMessage ItemPositionChangeMessage;
		ItemPositionChangeMessage.Instance = this;
		ItemPositionChangeMessage.OldPosition = OldPosition;
		ItemPositionChangeMessage.NewPosition = NewPosition;

		OnItemPositionChange.ExecuteIfBound(ItemPositionChangeMessage);
	}
	
	FOnItemPositionChange& OnItemPositionChangeDelegate()
	{
		return OnItemPositionChange;
	}
	
protected:
	UPROPERTY(BlueprintReadOnly, Category = "Grid Item Instance", ReplicatedUsing = OnRep_ItemPosition)
	FGridInvSys_ItemPosition ItemPosition;
	UFUNCTION()
	void OnRep_ItemPosition();

	UPROPERTY(BlueprintReadOnly, Category = "Grid Item Instance", Replicated)
	FGridInvSys_ItemPosition LastItemPosition;
	
	UPROPERTY()
	FGridInvSys_ItemPosition NewTempItemPosition;
	UPROPERTY()
	bool bIsFirstRepItemPosition = true;

private:
	FOnItemPositionChange OnItemPositionChange;
};
