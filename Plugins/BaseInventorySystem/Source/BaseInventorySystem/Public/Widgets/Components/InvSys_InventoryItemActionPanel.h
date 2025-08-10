// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InvSys_InventoryItemActionPanel.generated.h"

class UNamedSlot;
class UCanvasPanel;
class UInvSys_InventoryItemInstance;

/**
 * 
 */
UCLASS()
class BASEINVENTORYSYSTEM_API UInvSys_InventoryItemActionPanel : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;
	virtual void NativeOnCallOut(UInvSys_InventoryItemInstance* InItemInstance);
	virtual void NativeOnCallBack();
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	UFUNCTION(BlueprintImplementableEvent)
	void OnCallOut(UInvSys_InventoryItemInstance* InItemInstance);

	UFUNCTION(BlueprintImplementableEvent)
	void OnCallBack();

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Item Action List", meta = (BindWidget))
	TObjectPtr<UCanvasPanel> Canvas;

	UPROPERTY(BlueprintReadOnly, Category = "Item Action List", meta = (BindWidget))
	TObjectPtr<UNamedSlot> ActionList;

	UPROPERTY(BlueprintReadOnly, Category = "Item Action List")
	TWeakObjectPtr<UInvSys_InventoryItemInstance> ItemInstance;
};
