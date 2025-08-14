// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "InvSys_DragDropOperation.generated.h"

class UInvSys_InventoryItemInstance;

/**
 * 
 */
UCLASS()
class BASEINVENTORYSYSTEM_API UInvSys_DragDropOperation : public UDragDropOperation
{
	GENERATED_BODY()

public:
	template<class T = UInvSys_InventoryItemInstance>
	T* GetItemInstance() const
	{
		return (T*)Payload;
	}
};
