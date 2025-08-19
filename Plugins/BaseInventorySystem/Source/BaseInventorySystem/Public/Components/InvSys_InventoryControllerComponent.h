// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseInventorySystem.h"
#include "InvSys_InventoryComponent.h"
#include "InvSys_InventoryControllerComponent.generated.h"

class UInvSys_InventoryHUD;
class UInvSys_InventoryItemInstance;

UCLASS(Blueprintable, meta=(BlueprintSpawnableComponent))
class BASEINVENTORYSYSTEM_API UInvSys_InventoryControllerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInvSys_InventoryControllerComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void InitializeComponent() override;

	// todo::如果自定义 HUD，请重载该函数并添加自己的处理逻辑
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Player Inventory Component")
	virtual UInvSys_InventoryHUD* ConstructInventoryHUD();

	UInvSys_InventoryHUD* GetInventoryHUD() const;

	void CancelDragItemInstance();

public:
	/**
	 * RPC Functions
	 **/

	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Player Inventory Component")
	void Server_EquipItemDefinition(UInvSys_InventoryComponent* InvComp,TSubclassOf<UInvSys_InventoryItemDefinition> ItemDef, FGameplayTag SlotTag);
	
	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Player Inventory Component")
	void Server_EquipItemInstance(UInvSys_InventoryComponent* InvComp,UInvSys_InventoryItemInstance* ItemInstance, FGameplayTag InventoryTag);

	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Player Inventory Component")
	void Server_UnEquipItemInstanceByTag(UInvSys_InventoryComponent* InvComp, FGameplayTag InventoryTag);

	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Player Inventory Component")
	void Server_UnEquipItemInstance(UInvSys_InventoryItemInstance* ItemInstance);

	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Player Inventory Component")
	void Server_RemoveItemInstance(UInvSys_InventoryItemInstance* ItemInstance);

	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Player Inventory Component")
	void Server_SplitItemInstance(UInvSys_InventoryItemInstance* ItemInstance, int32 SplitSize);

	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Player Inventory Component")
	void Server_SwapItemInstance(UInvSys_InventoryItemInstance* FromItemInstance, UInvSys_InventoryItemInstance* ToItemInstance);

	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Player Inventory Component")
	void Server_PickupItemInstance(UInvSys_InventoryComponent* InvComp, AInvSys_PickableItems* PickableItems, bool bIsAutoEquip = true);

	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Player Inventory Component")
	void Server_DragItemInstance(UInvSys_InventoryComponent* InvComp, UInvSys_InventoryItemInstance* InItemInstance);

	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Player Inventory Component")
	void Server_CancelDragItemInstance(UInvSys_InventoryItemInstance* InItemInstance);

	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Player Inventory Component")
	void Server_DropItemInstanceToWorld(UInvSys_InventoryItemInstance* InItemInstance);

	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Player Inventory Component")
	void Server_SuperposeItemInstance(UInvSys_InventoryItemInstance* FromItemInstance, UInvSys_InventoryItemInstance* ToItemInstance);

	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Player Inventory Component")
	void Server_UpdateQuickBarItemReference(UInvSys_InventoryComponent* InvComp, FGameplayTag InventoryTag, UInvSys_InventoryItemInstance* ItemInstance, int32 Index);

	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Player Inventory Component")
	void Server_UseItemInstance(UInvSys_InventoryItemInstance* ItemInstance);
	
protected:
	bool HasAuthority() const;

	UFUNCTION(BlueprintPure, Category = "Player Inventory Component")
	APlayerController* GetPlayerController() const;

	void SetDraggingItemInstance(UInvSys_InventoryItemInstance* NewDragItemInstance);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_DraggingItemInstance, Category = "Player Inventory Component")
	TWeakObjectPtr<UInvSys_InventoryItemInstance> DraggingItemInstance = nullptr;
	UFUNCTION()
	void OnRep_DraggingItemInstance(const TWeakObjectPtr<UInvSys_InventoryItemInstance>& OldDraggingItemInstance);

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Player Inventory Component")
	TSubclassOf<UInvSys_InventoryHUD> InventoryHUDClass;

	UPROPERTY(BlueprintReadOnly, Category = "Player Inventory Component")
	UInvSys_InventoryHUD* InventoryHUD = nullptr;

private:
	UPROPERTY(BlueprintReadOnly, Category = "Player Inventory Component", meta = (AllowPrivateAccess))
	TObjectPtr<APlayerController> Controller;
};