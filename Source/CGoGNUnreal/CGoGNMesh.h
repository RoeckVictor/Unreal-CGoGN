// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "KismetProceduralMeshLibrary.h"

#include <CGoGNUnreal/cgogn/core/functions/cells.h>
#include <CGoGNUnreal/cgogn/core/functions/attributes.h>
#include <CGoGNUnreal/cgogn/core/functions/mesh_ops/face.h>
#include <CGoGNUnreal/cgogn/core/functions/traversals/global.h>
#include <CGoGNUnreal/cgogn/core/types/mesh_traits.h>

#include <CGoGNUnreal/cgogn/io/surface/off.h>

#include <CGoGNUnreal/cgogn/geometry/types/vector_traits.h>
#include <CGoGNUnreal/cgogn/geometry/algos/ear_triangulation.h>
#include <CGoGNUnreal/cgogn/geometry/algos/laplacian.h>

#include <CGoGNUnreal/cgogn/modeling/algos/subdivision.h>
#include <CGoGNUnreal/cgogn/modeling/algos/decimation/decimation.h>

#include "CGoGNMesh.generated.h"

using namespace cgogn;
using namespace geometry;
using namespace modeling;

using Mesh = CMap2;
using MyVec3 = geometry::Vec3;
using MyVertex = typename mesh_traits<Mesh>::Vertex;
using MyEdge = typename mesh_traits<Mesh>::Edge;
using MyFace = typename mesh_traits<Mesh>::Face;

UCLASS()
class CGOGNUNREAL_API ACGoGNMesh : public AActor
{
	GENERATED_BODY()
	
public:	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	// Mesh storing the result of MarchingCubes
	UProceduralMeshComponent* mesh;

	CMap2 cmap;

	// Sets default values for this actor's properties
	ACGoGNMesh();

	// Chargement d'un ficher OFF
	UFUNCTION(BlueprintCallable, Category = "CGoGN")
	bool LoadMeshFromFile(const FString filename);

	// Operations Geometriques
	UFUNCTION(BlueprintCallable, Category = "CGoGN")
	TArray<int> Laplacian(const FVector origin, const float radius);
	UFUNCTION(BlueprintCallable, Category = "CGoGN")
	TArray<int> TransfoFace(const int face, const FVector transfo_vector, int transfo_type, const float rayon, const int proportion_type);
	UFUNCTION(BlueprintCallable, Category = "CGoGN")
	TArray<int> TransfoEdge(const int edge, const FVector transfo_vector, int transfo_type, const float rayon, const int proportion_type);
	UFUNCTION(BlueprintCallable, Category = "CGoGN")
	TArray<int> TransfoVertex(const int vertex, const FVector transfo_vector, int transfo_type, const float rayon, const int proportion_type);

	// Operations Topologiques
	UFUNCTION(BlueprintCallable, Category = "CGoGN")
	void Subdivide(const FVector origin, const float radius);
	UFUNCTION(BlueprintCallable, Category = "CGoGN")
	void Simplify(const int nb_vertices, const float max_length);
	UFUNCTION(BlueprintCallable, Category = "CGoGN")
	int ExtrudeFace(const int face);
	UFUNCTION(BlueprintCallable, Category = "CGoGN")
	void CutEdge(const FVector origin, const float radius, const float max_length);

	UFUNCTION(BlueprintCallable, Category = "CGoGN")
	int ClosestVertice(const FVector origin, const float radius);
	UFUNCTION(BlueprintCallable, Category = "CGoGN")
	int ClosestEdge(const FVector origin, const float radius);
	UFUNCTION(BlueprintCallable, Category = "CGoGN")
	int ClosestFace(const FVector origin, const float radius);

	UFUNCTION(BlueprintCallable, Category = "CGoGN")
	TArray<FVector> GetVertices();
	UFUNCTION(BlueprintCallable, Category = "CGoGN")
	TArray<FVector> GetEdges();
	UFUNCTION(BlueprintCallable, Category = "CGoGN")
	TArray<FVector> GetFacesNormals();
	UFUNCTION(BlueprintCallable, Category = "CGoGN")
	void GetCellNums(int &vertices, int &edges, int &faces);
	UFUNCTION(BlueprintCallable, Category = "CGoGN")
	int GetBorderNum();
	UFUNCTION(BlueprintCallable, Category = "CGoGN")
	int GetCCNum();
	UFUNCTION(BlueprintCallable, Category = "CGoGN")
	void GetBoundingBox(FVector& start, FVector& end);

	UFUNCTION(BlueprintCallable, Category = "CGoGN")
	TArray<FVector> DartsPositions(float position_scale, float length_scale);

	// Utilite
	float GetScale(const float distance, const float rayon, const int proportion_type);
	FVector RotateAroundAxis(const FVector vertex, const FVector origin, const float angle, const int axis);
	void ComputeMeshArrays(TArray<FVector>& vertices, TArray<int32>& triangles, TArray<FVector2D>& UV0, TArray<FVector>& normals,
						   TArray<FProcMeshTangent>& tangents, TArray<FColor>& vertexColors);
	bool GenerateProceduralMesh();
	bool UpdateProceduralMesh();

	FVector PolygonCentroid(TArray<FVector> P);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
