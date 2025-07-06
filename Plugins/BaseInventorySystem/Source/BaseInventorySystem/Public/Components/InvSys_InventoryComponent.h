// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InvSys_InventoryComponent.generated.h"



// todo:: 定义装备新物品的代理

class UInvSys_PreEditInventoryObject;
class UInvSys_InventoryContentMapping;
class UInvSys_BaseInventoryObject;
class UInvSys_BaseEquipmentObject;
class UInvSys_BaseContainerObject;

struct FInvSys_InventoryItem;

/**
 * 
 */
UCLASS(Abstract, BlueprintType, meta=(BlueprintSpawnableComponent))
class BASEINVENTORYSYSTEM_API UInvSys_InventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UInvSys_InventoryComponent();
	
	/** 仅本地控制器，由用户手动调用。 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic)
	virtual void InitInventoryObj(APlayerController* NewPlayerController);

	UFUNCTION(Client, Reliable)
	virtual void Client_TryRefreshInventoryObject();

	/**
	 * 将物品添加到装备槽
	 * @param NewItem 
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory Component")
	virtual void AddInventoryItemToEquipSlot(const FInvSys_InventoryItem& NewItem);
	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	virtual void NativeOnInitInventoryObjects(APlayerController* InController);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCosmetic)
	void OnInitInventoryObjects(APlayerController* InController);
	
private:
	/**
	 * 构建库存列表，仅推荐在 Server 端调用
	 */
	void ConstructInventoryList();

public:
	/**
	 * Getter Or Setter
	 **/
	
	/** Returns true if the owner's role is ROLE_Authority */
	FORCEINLINE bool HasAuthority() const;

	FORCEINLINE bool IsLocalController() const;

	FORCEINLINE APlayerController* GetPlayerController() const;

	virtual bool IsContainsInventoryItem(const FName ItemUniqueID);
	
	UInvSys_PreEditInventoryObject* GetPreEditInventoryObject(int32 Index) const;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
protected:
	/** [Replication] 库存对象列表，由 InventoryContentMapping 管理，支持配置背包、装备槽或其他类型 */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Inventory Component")
	TArray<UInvSys_BaseInventoryObject*> InventoryObjectList;
	
	/** [CDO]库存内容映射，使用数据资产方便管理 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory Component")
	TObjectPtr<UInvSys_InventoryContentMapping> InventoryContentMapping;

	/** [Client & Server] 加速库存对象查询速度 */
	UPROPERTY(BlueprintReadOnly, Category = "Inventory Component")
	TMap<FName, UInvSys_BaseInventoryObject*> InventoryObjectMap;

	/** [Client] 拥有该组件的玩家控制器。 */
	UPROPERTY(BlueprintReadOnly, Category = "Inventory Component")
	APlayerController* OwningPlayer;

	/** [Client] 库存对象是否已经更新？限制初始化函数只执行一次。 */
	bool bIsInitInventoryObjects;
};
