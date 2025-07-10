// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "InventoryObject/InvSys_BaseEquipmentObject.h"
#include "InventoryObject/InvSys_BaseInventoryObject.h"
#include "InvSys_InventoryComponent.generated.h"



// todo:: 定义装备新物品的代理

class UInvSys_InventoryLayoutWidget;
class UInvSys_InventoryItemDefinition;
class UInvSys_PreEditInventoryObject;
class UInvSys_BaseInventoryObject;
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
	UInvSys_InventoryComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	/** 仅本地控制器，由用户手动调用。===> CreateDisplayWidget */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic)
	virtual UInvSys_InventoryLayoutWidget* CreateDisplayWidget(APlayerController* NewPlayerController);

	UFUNCTION(Client, Reliable)
	virtual void Client_TryRefreshInventoryObject();

	/**
	 * 将物品添加到装备槽
	 * @param NewItem 
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory Component", meta = (DeprecatedFunction))
	virtual void AddInventoryItemToEquipSlot(const FInvSys_InventoryItem& NewItem);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Inventory Component")
	void EquipInventoryItem(TSubclassOf<UInvSys_InventoryItemDefinition> ItemDefinition, FGameplayTag SlotTag);
	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	/** 构建库存对象列表 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Inventory Component")
	void ConstructInventoryObjects();

	/** 创建所有库存对象后被调用 */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCosmetic, Category = "Inventory Component")
	void OnConstructInventoryObjects();

public:
	/**
	 * Getter Or Setter
	 **/

	UFUNCTION(BlueprintCallable, meta = (DeterminesOutputType = OutClass))
	UInvSys_BaseInventoryObject* GetInventoryObject(FGameplayTag Tag, TSubclassOf<UInvSys_BaseInventoryObject> OutClass) const;

	template<class T = UInvSys_BaseInventoryObject>
	T* GetInventoryObject(FGameplayTag Tag) const
	{
		if (InventoryObjectMap.Contains(Tag) && InventoryObjectMap[Tag]->IsA<T>())
		{
			return Cast<T>(InventoryObjectMap[Tag]);
		}
		return nullptr;
	}
	
	/** Returns true if the owner's role is ROLE_Authority */
	FORCEINLINE bool HasAuthority() const;

	FORCEINLINE bool IsLocalController() const;

	FORCEINLINE APlayerController* GetPlayerController() const;

	virtual bool IsContainsInventoryItem(const FName ItemUniqueID);
	
	UInvSys_PreEditInventoryObject* GetPreEditInventoryObject(int32 Index) const;

	//~UObject interface
	virtual bool ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags) override;
	virtual void ReadyForReplication() override;
	//~End of UObject interface
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
protected:
	/** [Replication] 库存对象列表，由 InventoryContentMapping 管理，支持配置背包、装备槽或其他类型 */
	UPROPERTY(ReplicatedUsing = OnRep_InventoryObjectList, BlueprintReadOnly, Category = "Inventory Component")
	TArray<UInvSys_BaseInventoryObject*> InventoryObjectList;
	UFUNCTION()
	void OnRep_InventoryObjectList();

	/** [Client & Server] 加速库存对象查询速度 */
	UPROPERTY()
	TMap<FGameplayTag, UInvSys_BaseInventoryObject*> InventoryObjectMap;
	
	/** [CDO]库存内容映射，使用数据资产方便管理 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory Component")
	TObjectPtr<class UInvSys_InventoryContentMapping> InventoryContentMapping;

	/** [Client] 拥有该组件的玩家控制器。 */
	UPROPERTY(BlueprintReadOnly, Category = "Inventory Component")
	APlayerController* OwningPlayer;

	/** [Client] 库存对象是否已经更新？限制初始化函数只执行一次。 */
	bool bDisplayWidgetIsValid;

	/** 库存组件的展示菜单 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory Component")
	TSubclassOf<UInvSys_InventoryLayoutWidget> LayoutWidgetClass;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory Component")
	TObjectPtr<UInvSys_InventoryLayoutWidget> LayoutWidget;
};
