// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseInventorySystem.h"
#include "GridInvSys_CommonType.h"
#include "Components/InvSys_InventoryComponent.h"
#include "Data/InvSys_InventoryItemInstance.h"
#include "GridInvSys_InventoryItemInstance.generated.h"

class UGridInvSys_ContainerGridWidget;

USTRUCT(BlueprintType)
struct FGridInvSys_ItemPositionChangeMessage{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Message")
	UInvSys_InventoryItemInstance* ItemInstance = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Message")
	UInvSys_InventoryComponent* InventoryComponent = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Message")
	FGridInvSys_ItemPosition OldPosition;

	UPROPERTY(BlueprintReadOnly, Category = "Message")
	FGridInvSys_ItemPosition NewPosition;
};

/**
 * 
 */
UCLASS()
class GRIDINVENTORYSYSTEM_API UGridInvSys_InventoryItemInstance : public UInvSys_InventoryItemInstance
{
	GENERATED_BODY()

public:
	virtual void PostReplicatedChange() override;

	virtual void OnTransferItems(UInvSys_InventoryModule_Container* ContainerFragment) override;
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void InitItemInstanceProps(const FGridInvSys_ItemPosition& NewItemPosition, bool bIsBroadcast);

	void SetItemPosition(const FGridInvSys_ItemPosition& NewItemPosition, bool bIsBroadcast = true);

	FGridInvSys_ItemPosition GetItemPosition() const
	{
		return ItemPosition;
	}

	FGridInvSys_ItemPosition GetLastItemPosition() const
	{
		return LastItemPosition;
	}

	FIntPoint GetItemSize();
	FIntPoint GetItemSize(EGridInvSys_ItemDirection Direction);
	FIntPoint GetItemDefaultSize();

	void BroadcastItemPositionChangeMessage(const FGridInvSys_ItemPosition& OldPosition, const FGridInvSys_ItemPosition& NewPosition);
	
protected:
	UPROPERTY(BlueprintReadOnly, Category = "Grid Item Instance", ReplicatedUsing = OnRep_ItemPosition)
	FGridInvSys_ItemPosition ItemPosition;
	UFUNCTION()
	void OnRep_ItemPosition(const FGridInvSys_ItemPosition& OldItemPosition);
	void Execute_ItemPosition(const FGridInvSys_ItemPosition& OldItemPosition);

	UPROPERTY(BlueprintReadOnly, Category = "Grid Item Instance"/*, Replicated*/)
	FGridInvSys_ItemPosition LastItemPosition;
	
private:
	bool bWaitPostRepNotify_ItemPosition = false;
};
