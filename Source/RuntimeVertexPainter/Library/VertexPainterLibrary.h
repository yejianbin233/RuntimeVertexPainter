// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "VertexPainterLibrary.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(VertexPainterBlueprintFunctionLibraryLog, Log, All)


USTRUCT(BlueprintType)
struct FVertexOverrideColorInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	int32 VertexIndex;

	UPROPERTY(BlueprintReadWrite)
	FColor OverrideColor;
};

/**
 * 
 */
UCLASS()
class RUNTIMEVERTEXPAINTER_API UVertexPainterLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	/* 视频教程所实现的代码 - 没有进行太多的调整
	 * @description: PaintVertexColorByIndex - 根据顶点索引给顶点绘制顶点颜色
	 * @param
	 *	StaticMeshComponent - 被绘制的静态网格体
	 *	Color - 绘制颜色
	 *	Index - 顶点索引
	 *	LODIndex - 不同 LOD 的顶点索引
	 * 
	 * @author: yejianbin
	 * @version: v1.0
	 * @createTime: 2022年09月19日 星期一 20:09:27
	 */
	UFUNCTION(BlueprintCallable, Category="Vertex", DisplayName="根据索引绘制顶点颜色")
	static void PaintVertexColorByIndex(UStaticMeshComponent* StaticMeshComponent, FLinearColor Color, int32 Index, int32 LODIndex);

	/*
	 * @description: PaintVertexColorInSphere - 在球上绘制顶点颜色
	 * @param
	 * StaticMeshComponent - 被绘制的静态网格体(球体)
	 * Color - 绘制颜色
	 * Radius - 绘制半径
	 * Position - 绘制位置
	 * 
	 * @author: yejianbin
	 * @version: v1.0
	 * @createTime: 2022年09月19日 星期一 20:09:02
	 */
	UFUNCTION(BlueprintCallable, Category="Vertex", DisplayName="在球上绘制顶点颜色")
	static void PaintVertexColorInSphere(UStaticMeshComponent* StaticMeshComponent, FLinearColor Color, float Radius, FVector Position);

	// 蓝图不需要调用 - GetStaticMeshVertexColors 和 GetSkeletalMeshVertexColors
	UFUNCTION(Category="Vertex", DisplayName="根据 LOD 索引获取静态网格体组件的顶点颜色数组")
	static TArray<FColor> GetStaticMeshVertexColors(UStaticMeshComponent* StaticMeshComponent, int32 LODIndex);

	UFUNCTION(Category="Vertex", DisplayName="根据 LOD 索引获取骨骼网格体组件的顶点颜色数组")
	static TArray<FColor> GetSkeletalMeshVertexColors(USkeletalMeshComponent* SkeletalMeshComponent, int32 LODIndex);

	UFUNCTION(BlueprintCallable, Category="Vertex", DisplayName="覆盖静态网格体的顶点颜色")
	static void OverrideStaticMeshVertexColor(UStaticMeshComponent* StaticMeshComponent, int32 LODIndex, TArray<FVertexOverrideColorInfo> VertexOverrideColorInfos);

	UFUNCTION(BlueprintCallable, Category="Vertex", DisplayName="覆盖骨骼网格体的顶点颜色")
	static void OverrideSkeletalMeshVertexColor(USkeletalMeshComponent* SkeletalMeshComponent, int32 LODIndex, TArray<FVertexOverrideColorInfo> VertexOverrideColorInfos);

	UFUNCTION(BlueprintCallable, Category="Vertex", DisplayName="获取球体内的静态网格体顶点覆盖颜色信息数组")
	static TArray<FVertexOverrideColorInfo> GetStaticMeshVertexOverrideColorInfoInSphere(UStaticMeshComponent* StaticMeshComponent, int32 LODIndex, FVector SphereWorldPosition, float Radius, FLinearColor OverrideColor);

	UFUNCTION(BlueprintCallable, Category="Vertex", DisplayName="获取球体内的骨骼网格体顶点覆盖颜色信息数组")
	static TArray<FVertexOverrideColorInfo> GetSkeletalMeshVertexOverrideColorInfoInSphere(USkeletalMeshComponent* SkeletalMeshComponent, int32 LODIndex, FVector SphereWorldPosition, float Radius, FLinearColor OverrideColor);

	// 拓展 - 可参照 GetVertexOverrideColorInfoInSphere，进行其他方式运算来得到所需要覆盖颜色的顶点数据，如：正方体、长方体等形状
	// TODO 设想：是否可以通过屏幕图片来进行绘制
	// 1. 简单模式：将一个图片绘制到一个平面网格体上
	// 2. 困难模式：将一个图片绘制到不规则形状上
	// 3. 究极困难模式：将一个图片绘制到不规则形状上，并且如果图片覆盖范围比不规则形状广，那么是否可以模拟泼洒的效果
};
