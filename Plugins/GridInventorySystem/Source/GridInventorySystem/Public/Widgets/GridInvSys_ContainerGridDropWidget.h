// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GridInvSys_InventoryWidget.h"
#include "GridInvSys_ContainerGridDropWidget.generated.h"

class USizeBox;
/**
 * 
 */
UCLASS()
class GRIDINVENTORYSYSTEM_API UGridInvSys_ContainerGridDropWidget : public UGridInvSys_InventoryWidget
{
	GENERATED_BODY()

public:
	void NativeOnDraggingHovered(bool bIsCanDrop);

	void NativeOnEndDraggingHovered();
	
protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "DraggingDisplayWidget")
	void OnDraggingHovered(bool bIsCanDrop);

	UFUNCTION(BlueprintImplementableEvent, Category = "DraggingDisplayWidget")
	void OnEndDraggingHovered();

	virtual void NativePreConstruct() override;

	virtual void NativeConstruct() override;

protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<USizeBox> DraggingDisplayGridItem; 
};
